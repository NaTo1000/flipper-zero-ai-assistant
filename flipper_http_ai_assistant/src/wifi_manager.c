#include "../inc/wifi_manager.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <furi_hal_serial.h>
#include <storage/storage.h>
#include <string.h>
#include <stdio.h>

/* FlipperHTTP serial command templates */
#define FHTTP_BAUD        115200
#define FHTTP_TIMEOUT_MS  5000
#define CRED_PATH         "/ext/apps/GPIO/flipperhttp_wifi.cfg"
#define WIFI_SCAN_CMD     "[WIFI/SCAN]\r\n"
#define MAX_SCAN_RESULTS  16

/* ── Alloc / free ─────────────────────────────────────────────────────── */

WiFiManager* wifi_manager_alloc() {
    WiFiManager* m = malloc(sizeof(WiFiManager));
    if(m) {
        memset(m, 0, sizeof(WiFiManager));
        m->state = WiFiStateDisconnected;
    }
    return m;
}

void wifi_manager_free(WiFiManager* mgr) {
    if(!mgr) return;
    if(mgr->scan_results) {
        for(uint8_t i = 0; i < mgr->scan_count; i++) free(mgr->scan_results[i]);
        free(mgr->scan_results);
    }
    free(mgr);
}

/* ── Serial helpers ───────────────────────────────────────────────────── */

static FuriHalSerialHandle* serial_open() {
    FuriHalSerialHandle* h = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(h) furi_hal_serial_init(h, FHTTP_BAUD);
    return h;
}

static void serial_close(FuriHalSerialHandle* h) {
    if(h) {
        furi_hal_serial_deinit(h);
        furi_hal_serial_control_release(h);
    }
}

/* Send command and wait for a line containing `expect`. */
static bool serial_cmd(
    FuriHalSerialHandle* h,
    const char* cmd,
    const char* expect,
    char* out_buf,
    size_t out_size,
    uint32_t timeout_ms) {
    furi_hal_serial_tx(h, (const uint8_t*)cmd, strlen(cmd));
    furi_hal_serial_tx_wait_complete(h);

    uint8_t buf[256];
    size_t len = 0;
    uint32_t t0 = furi_get_tick();

    while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(timeout_ms)) {
        if(furi_hal_serial_async_rx_available(h)) {
            uint8_t b = furi_hal_serial_async_rx(h);
            if(len < sizeof(buf) - 1) buf[len++] = b;
            if(b == '\n') {
                buf[len] = '\0';
                if(strstr((char*)buf, expect)) {
                    if(out_buf && out_size > 0) {
                        snprintf(out_buf, out_size, "%.*s", (int)len, buf);
                    }
                    return true;
                }
                len = 0;
            }
        }
        furi_delay_ms(5);
    }
    return false;
}

/* ── Public API ───────────────────────────────────────────────────────── */

bool wifi_manager_connect(WiFiManager* mgr, const char* ssid, const char* password) {
    if(!mgr || !ssid || !password) return false;
    snprintf(mgr->ssid, sizeof(mgr->ssid), "%s", ssid);
    snprintf(mgr->password, sizeof(mgr->password), "%s", password);
    mgr->state = WiFiStateConnecting;

    FuriHalSerialHandle* h = serial_open();
    if(!h) {
        mgr->state = WiFiStateError;
        return false;
    }

    char cmd[160];
    snprintf(
        cmd,
        sizeof(cmd),
        "[WIFI/CONNECT]{\"ssid\":\"%s\",\"password\":\"%s\"}\r\n",
        ssid,
        password);

    char resp[64];
    bool ok = serial_cmd(h, cmd, "[WIFI/CONNECTED]", resp, sizeof(resp), FHTTP_TIMEOUT_MS);
    serial_close(h);

    mgr->state = ok ? WiFiStateConnected : WiFiStateError;
    if(ok) wifi_manager_get_ip(mgr);
    return ok;
}

bool wifi_manager_disconnect(WiFiManager* mgr) {
    if(!mgr) return false;

    FuriHalSerialHandle* h = serial_open();
    if(!h) return false;

    bool ok = serial_cmd(h, "[WIFI/DISCONNECT]\r\n", "[WIFI/DISCONNECTED]", NULL, 0, 3000);
    serial_close(h);

    mgr->state = WiFiStateDisconnected;
    memset(mgr->ip_address, 0, sizeof(mgr->ip_address));
    return ok;
}

bool wifi_manager_scan(WiFiManager* mgr) {
    if(!mgr) return false;
    mgr->scan_in_progress = true;

    FuriHalSerialHandle* h = serial_open();
    if(!h) {
        mgr->scan_in_progress = false;
        return false;
    }

    /* Free previous results */
    if(mgr->scan_results) {
        for(uint8_t i = 0; i < mgr->scan_count; i++) free(mgr->scan_results[i]);
        free(mgr->scan_results);
        mgr->scan_results = NULL;
        mgr->scan_count = 0;
    }

    furi_hal_serial_tx(h, (const uint8_t*)WIFI_SCAN_CMD, strlen(WIFI_SCAN_CMD));
    furi_hal_serial_tx_wait_complete(h);

    /* Collect SSID lines until [WIFI/SCAN/END] */
    uint8_t buf[128];
    size_t len = 0;
    uint32_t t0 = furi_get_tick();

    /* Allocate up to MAX_SCAN_RESULTS results */
    mgr->scan_results = malloc(MAX_SCAN_RESULTS * sizeof(char*));
    memset(mgr->scan_results, 0, MAX_SCAN_RESULTS * sizeof(char*));

    while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(10000)) {
        if(furi_hal_serial_async_rx_available(h)) {
            uint8_t b = furi_hal_serial_async_rx(h);
            if(len < sizeof(buf) - 1) buf[len++] = b;
            if(b == '\n') {
                buf[len] = '\0';
                char* line = (char*)buf;
                /* Trim CR/LF */
                while(len > 0 && (buf[len - 1] == '\r' || buf[len - 1] == '\n')) {
                    buf[--len] = '\0';
                }
                if(strstr(line, "[WIFI/SCAN/END]")) break;
                if(strstr(line, "[WIFI/SCAN/RESULT]")) {
                    /* Format: [WIFI/SCAN/RESULT]{"ssid":"NAME",...} */
                    char* ssid_start = strstr(line, "\"ssid\":\"");
                    if(ssid_start && mgr->scan_count < MAX_SCAN_RESULTS) {
                        ssid_start += 8;
                        char* ssid_end = strchr(ssid_start, '"');
                        if(ssid_end) {
                            size_t ssid_len = ssid_end - ssid_start;
                            mgr->scan_results[mgr->scan_count] = malloc(ssid_len + 1);
                            if(mgr->scan_results[mgr->scan_count]) {
                                memcpy(mgr->scan_results[mgr->scan_count], ssid_start, ssid_len);
                                mgr->scan_results[mgr->scan_count][ssid_len] = '\0';
                                mgr->scan_count++;
                            }
                        }
                    }
                }
                len = 0;
            }
        }
        furi_delay_ms(5);
    }

    serial_close(h);
    mgr->scan_in_progress = false;
    return mgr->scan_count > 0;
}

bool wifi_manager_get_ip(WiFiManager* mgr) {
    if(!mgr) return false;

    FuriHalSerialHandle* h = serial_open();
    if(!h) return false;

    char resp[64];
    bool ok = serial_cmd(h, "[WIFI/IP]\r\n", "[WIFI/IP/RESULT]", resp, sizeof(resp), 3000);
    serial_close(h);

    if(ok) {
        /* Parse IP from: [WIFI/IP/RESULT]{"ip":"x.x.x.x"} */
        char* ip_start = strstr(resp, "\"ip\":\"");
        if(ip_start) {
            ip_start += 6;
            char* ip_end = strchr(ip_start, '"');
            if(ip_end) {
                size_t ip_len = ip_end - ip_start;
                if(ip_len < sizeof(mgr->ip_address)) {
                    memcpy(mgr->ip_address, ip_start, ip_len);
                    mgr->ip_address[ip_len] = '\0';
                }
            }
        }
    }
    return ok;
}

bool wifi_manager_ping(WiFiManager* mgr, const char* host) {
    if(!mgr || !host) return false;

    FuriHalSerialHandle* h = serial_open();
    if(!h) return false;

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "[WIFI/PING]{\"host\":\"%s\"}\r\n", host);

    bool ok = serial_cmd(h, cmd, "[WIFI/PING/OK]", NULL, 0, 5000);
    serial_close(h);
    return ok;
}

bool wifi_manager_save_credentials(WiFiManager* mgr) {
    if(!mgr) return false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    bool ok = false;
    if(storage_file_open(file, CRED_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        char buf[160];
        int len = snprintf(buf, sizeof(buf), "ssid=%s\npassword=%s\n", mgr->ssid, mgr->password);
        ok = storage_file_write(file, buf, len) == (size_t)len;
        storage_file_close(file);
    }

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

void wifi_manager_draw_status(Canvas* canvas, WiFiManager* mgr) {
    ui_draw_header(canvas, "WiFi Manager");
    if(!mgr) return;

    canvas_set_font(canvas, FontSecondary);

    const char* state_str = "Disconnected";
    switch(mgr->state) {
    case WiFiStateConnecting:
        state_str = "Connecting...";
        break;
    case WiFiStateConnected:
        state_str = "Connected";
        break;
    case WiFiStateError:
        state_str = "Error";
        break;
    default:
        break;
    }

    canvas_draw_str(canvas, 4, 22, "Status:");
    canvas_draw_str(canvas, 52, 22, state_str);

    if(mgr->ssid[0]) {
        canvas_draw_str(canvas, 4, 32, "SSID:");
        canvas_draw_str(canvas, 40, 32, mgr->ssid);
    }
    if(mgr->ip_address[0]) {
        canvas_draw_str(canvas, 4, 42, "IP:");
        canvas_draw_str(canvas, 22, 42, mgr->ip_address);
    }
    if(mgr->scan_in_progress) {
        canvas_draw_str(canvas, 4, 52, "Scanning...");
    }
    ui_draw_info_hint(canvas);
}
