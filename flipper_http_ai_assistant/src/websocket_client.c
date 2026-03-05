#include "../inc/websocket_client.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <furi_hal_serial.h>
#include <string.h>
#include <stdio.h>

/*
 * WebSocket client implemented over FlipperHTTP serial protocol.
 * Commands:
 *   [WS/CONNECT]{"url":"wss://..."}  → [WS/CONNECTED]
 *   [WS/DISCONNECT]                  → [WS/DISCONNECTED]
 * Incoming messages arrive as:
 *   [WS/MESSAGE]{"data":"..."}
 */

#define WS_TIMEOUT_MS     8000
#define WS_BAUD           115200
#define WS_DISCONNECT_CMD "[WS/DISCONNECT]\r\n"

WebSocketClient* websocket_client_alloc() {
    WebSocketClient* c = malloc(sizeof(WebSocketClient));
    if(c) {
        memset(c, 0, sizeof(WebSocketClient));
        c->state = WebSocketStateClosed;
    }
    return c;
}

void websocket_client_free(WebSocketClient* client) {
    if(client) free(client);
}

bool websocket_client_start(WebSocketClient* client, const char* url) {
    if(!client || !url) return false;

    snprintf(client->url, sizeof(client->url), "%s", url);
    client->state = WebSocketStateConnecting;

    FuriHalSerialHandle* h = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!h) {
        client->state = WebSocketStateError;
        return false;
    }
    furi_hal_serial_init(h, WS_BAUD);

    char cmd[320];
    snprintf(cmd, sizeof(cmd), "[WS/CONNECT]{\"url\":\"%s\"}\r\n", url);
    furi_hal_serial_tx(h, (const uint8_t*)cmd, strlen(cmd));
    furi_hal_serial_tx_wait_complete(h);

    uint8_t rx_buf[128];
    size_t rx_len = 0;
    uint32_t t0 = furi_get_tick();
    bool connected = false;

    while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(WS_TIMEOUT_MS)) {
        if(furi_hal_serial_async_rx_available(h)) {
            uint8_t b = furi_hal_serial_async_rx(h);
            if(rx_len < sizeof(rx_buf) - 1) rx_buf[rx_len++] = b;
            if(b == '\n') {
                rx_buf[rx_len] = '\0';
                if(strstr((char*)rx_buf, "[WS/CONNECTED]")) {
                    connected = true;
                    break;
                }
                if(strstr((char*)rx_buf, "[WS/ERROR]")) break;
                rx_len = 0;
            }
        }
        furi_delay_ms(5);
    }

    furi_hal_serial_deinit(h);
    furi_hal_serial_control_release(h);

    client->state = connected ? WebSocketStateOpen : WebSocketStateError;
    return connected;
}

bool websocket_client_stop(WebSocketClient* client) {
    if(!client) return false;

    FuriHalSerialHandle* h = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!h) return false;
    furi_hal_serial_init(h, WS_BAUD);

    furi_hal_serial_tx(h, (const uint8_t*)WS_DISCONNECT_CMD, strlen(WS_DISCONNECT_CMD));
    furi_hal_serial_tx_wait_complete(h);

    /* Wait briefly for ack */
    uint8_t rx_buf[64];
    size_t rx_len = 0;
    uint32_t t0 = furi_get_tick();
    while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(2000)) {
        if(furi_hal_serial_async_rx_available(h)) {
            uint8_t b = furi_hal_serial_async_rx(h);
            if(rx_len < sizeof(rx_buf) - 1) rx_buf[rx_len++] = b;
            if(b == '\n') {
                rx_buf[rx_len] = '\0';
                if(strstr((char*)rx_buf, "[WS/DISCONNECTED]")) break;
                rx_len = 0;
            }
        }
        furi_delay_ms(5);
    }

    furi_hal_serial_deinit(h);
    furi_hal_serial_control_release(h);

    client->state = WebSocketStateClosed;
    return true;
}

void websocket_client_draw_status(Canvas* canvas, WebSocketClient* client) {
    ui_draw_header(canvas, "WebSocket Client");
    if(!client) return;

    canvas_set_font(canvas, FontSecondary);

    const char* state_str = "Closed";
    switch(client->state) {
    case WebSocketStateConnecting:
        state_str = "Connecting...";
        break;
    case WebSocketStateOpen:
        state_str = "Open";
        break;
    case WebSocketStateError:
        state_str = "Error";
        break;
    default:
        break;
    }

    canvas_draw_str(canvas, 4, 22, "State:");
    canvas_draw_str(canvas, 44, 22, state_str);

    if(client->url[0]) {
        /* Truncate URL to fit */
        char url_short[28];
        snprintf(url_short, sizeof(url_short), "%.27s", client->url);
        canvas_draw_str(canvas, 4, 32, "URL:");
        canvas_draw_str(canvas, 30, 32, url_short);
    }

    char msg_count[24];
    snprintf(msg_count, sizeof(msg_count), "Msgs: %lu", (unsigned long)client->messages_received);
    canvas_draw_str(canvas, 4, 42, msg_count);

    if(client->last_message[0]) {
        char msg_preview[28];
        snprintf(msg_preview, sizeof(msg_preview), "%.27s", client->last_message);
        canvas_draw_str(canvas, 4, 52, msg_preview);
    }

    ui_draw_info_hint(canvas);
}
