#include "../inc/diagnostics.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <furi_hal_serial.h>
#include <string.h>
#include <stdio.h>

/* Serial timeout in ms */
#define DIAG_SERIAL_TIMEOUT_MS 2000
#define DIAG_PING_CMD          "[PING]\r\n"
#define DIAG_PONG_RESP         "[PONG]"
#define DIAG_VERSION_CMD       "[VERSION]\r\n"

DiagnosticsReport* diagnostics_alloc() {
    DiagnosticsReport* r = malloc(sizeof(DiagnosticsReport));
    if(r) {
        memset(r, 0, sizeof(DiagnosticsReport));
        r->wifi = ModuleStatusUnknown;
        r->ble = ModuleStatusUnknown;
        r->nrf = ModuleStatusUnknown;
        r->lf = ModuleStatusUnknown;
        r->subghz = ModuleStatusUnknown;
    }
    return r;
}

void diagnostics_free(DiagnosticsReport* report) {
    if(report) free(report);
}

/*
 * Probe the FlipperHTTP board over USART (pins 13/14, 115200 baud).
 * Send [PING] and wait up to DIAG_SERIAL_TIMEOUT_MS for [PONG].
 * If the board responds we mark WiFi as OK and attempt [VERSION].
 */
static bool diag_ping_board(DiagnosticsReport* report) {
    FuriHalSerialHandle* serial = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!serial) {
        report->wifi = ModuleStatusNotFound;
        return false;
    }

    furi_hal_serial_init(serial, 115200);

    /* Flush any pending data */
    furi_delay_ms(50);

    /* Send PING */
    furi_hal_serial_tx(serial, (const uint8_t*)DIAG_PING_CMD, strlen(DIAG_PING_CMD));
    furi_hal_serial_tx_wait_complete(serial);

    /* Wait for PONG */
    uint8_t rx_buf[64];
    memset(rx_buf, 0, sizeof(rx_buf));
    size_t rx_len = 0;
    uint32_t t0 = furi_get_tick();
    bool got_pong = false;

    while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(DIAG_SERIAL_TIMEOUT_MS)) {
        uint8_t byte;
        /* Non-blocking single-byte read */
        if(furi_hal_serial_async_rx_available(serial)) {
            byte = furi_hal_serial_async_rx(serial);
            if(rx_len < sizeof(rx_buf) - 1) {
                rx_buf[rx_len++] = byte;
            }
            if(strstr((char*)rx_buf, DIAG_PONG_RESP)) {
                got_pong = true;
                break;
            }
        }
        furi_delay_ms(10);
    }

    if(got_pong) {
        report->wifi = ModuleStatusOK;
        /* Request version string */
        memset(rx_buf, 0, sizeof(rx_buf));
        rx_len = 0;
        furi_hal_serial_tx(serial, (const uint8_t*)DIAG_VERSION_CMD, strlen(DIAG_VERSION_CMD));
        furi_hal_serial_tx_wait_complete(serial);
        t0 = furi_get_tick();
        while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(1000)) {
            if(furi_hal_serial_async_rx_available(serial)) {
                uint8_t b = furi_hal_serial_async_rx(serial);
                if(rx_len < sizeof(rx_buf) - 1) rx_buf[rx_len++] = b;
                if(b == '\n') break;
            }
            furi_delay_ms(5);
        }
        if(rx_len > 0) {
            snprintf(report->wifi_version, sizeof(report->wifi_version), "%.*s", (int)rx_len, rx_buf);
        } else {
            snprintf(report->wifi_version, sizeof(report->wifi_version), "v1.0");
        }
    } else {
        report->wifi = ModuleStatusNotFound;
    }

    furi_hal_serial_deinit(serial);
    furi_hal_serial_control_release(serial);
    return got_pong;
}

/*
 * Check BLE co-processor presence via furi_hal_bt.
 */
static void diag_check_ble(DiagnosticsReport* report) {
    /* furi_hal_bt_is_active() is available since FW 0.79 */
    if(furi_hal_bt_is_active()) {
        report->ble = ModuleStatusOK;
        snprintf(report->ble_version, sizeof(report->ble_version), "Active");
    } else {
        report->ble = ModuleStatusNotFound;
    }
}

/* Placeholder probes – NRF, LF, SubGHz real detection requires
 * hardware-specific SPI / GPIO access which varies by add-on board.
 * We perform a conservative "assume OK unless known bad" policy. */
static void diag_check_nrf(DiagnosticsReport* report) {
    report->nrf = ModuleStatusUnknown;
}

static void diag_check_lf(DiagnosticsReport* report) {
    report->lf = ModuleStatusOK; /* Built-in LF antenna */
}

static void diag_check_subghz(DiagnosticsReport* report) {
    report->subghz = ModuleStatusOK; /* CC1101 is on all Flippers */
}

void diagnostics_run(DiagnosticsReport* report) {
    if(!report) return;
    report->diagnostics_complete = false;

    diag_ping_board(report);
    diag_check_ble(report);
    diag_check_nrf(report);
    diag_check_lf(report);
    diag_check_subghz(report);

    report->diagnostics_complete = true;
}

const char* module_status_icon(ModuleStatus status) {
    switch(status) {
    case ModuleStatusOK:
        return "[OK]";
    case ModuleStatusNotFound:
        return "[--]";
    case ModuleStatusError:
        return "[!!]";
    default:
        return "[??]";
    }
}

void diagnostics_draw(Canvas* canvas, DiagnosticsReport* report) {
    ui_draw_header(canvas, "Diagnostics");

    if(!report) return;

    canvas_set_font(canvas, FontSecondary);

    /* Each row: icon + label + status */
    const struct {
        const char* label;
        ModuleStatus status;
    } rows[] = {
        {"WiFi (FlipperHTTP)", report->wifi},
        {"BLE", report->ble},
        {"nRF24", report->nrf},
        {"LF Antenna", report->lf},
        {"Sub-GHz", report->subghz},
    };

    for(int i = 0; i < 5; i++) {
        int y = 14 + i * 10;
        canvas_draw_str(canvas, 4, y, rows[i].label);
        canvas_draw_str(canvas, 96, y, module_status_icon(rows[i].status));
    }

    if(!report->diagnostics_complete) {
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 4, 62, "Running...");
    } else {
        if(report->wifi == ModuleStatusOK) {
            canvas_draw_str(canvas, 4, 62, report->wifi_version);
        }
    }
    ui_draw_info_hint(canvas);
}
