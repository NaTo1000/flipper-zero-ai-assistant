#include "../inc/info_overlay.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>

/* Context-sensitive help texts shown when the user holds OK */
static const char* const info_texts[InfoContextCount] = {
    /* InfoContextSplash */
    "Welcome to FlipperHTTP AI Assistant!\n"
    "This app connects your Flipper Zero to WiFi using the FlipperHTTP "
    "ESP32 board. It offers HTTP requests, WebSocket, AI-guided setup, "
    "and a built-in tutorial manual.\n"
    "Press any key to continue to the startup diagnostics.",

    /* InfoContextDiagnostics */
    "Diagnostics checks all hardware modules:\n"
    "- WiFi: pings the FlipperHTTP board over USART\n"
    "- BLE: checks the built-in co-processor\n"
    "- nRF24: reads SPI CONFIG register\n"
    "- LF: verifies built-in 125 kHz antenna\n"
    "- Sub-GHz: checks CC1101 transceiver\n"
    "Status: [OK]=working, [--]=not found, [!!]=error",

    /* InfoContextMainMenu */
    "Main Menu options:\n"
    "WiFi Manager - Connect to a WiFi network\n"
    "AI Setup - Automated WiFi configuration wizard\n"
    "HTTP Client - Send HTTP GET/POST/PUT/DELETE\n"
    "WebSocket - Open a persistent WebSocket connection\n"
    "BLE Manager - Control Bluetooth LE\n"
    "nRF24 - Configure nRF24L01+ add-on module\n"
    "LF RFID - Read 125 kHz RFID tags\n"
    "Sub-GHz - View CC1101 radio status\n"
    "Manual - Tutorial and reference guide\n"
    "Diagnostics - Re-run hardware checks",

    /* InfoContextWifi */
    "WiFi Manager - Connect Flipper Zero to a WiFi network via the "
    "FlipperHTTP ESP32 board.\n"
    "UP/DOWN: scroll network list\n"
    "OK: select network / connect\n"
    "BACK: return to main menu\n"
    "Credentials are saved to internal storage and loaded on next launch.",

    /* InfoContextBle */
    "BLE Manager controls Flipper Zero's built-in Bluetooth LE radio.\n"
    "Enable starts advertising so the Flipper appears to nearby devices.\n"
    "Note: BLE is shared with the USB serial bridge; heavy BLE use may "
    "slow USART communication with the FlipperHTTP board.",

    /* InfoContextNrf */
    "nRF24L01+ Manager requires an nRF24 add-on board connected to the "
    "GPIO header. Default channel is 76 (2.476 GHz).\n"
    "Detection reads the CONFIG register over SPI.\n"
    "This module is NOT built into Flipper Zero.",

    /* InfoContextLf */
    "LF RFID reads 125 kHz cards (EM4100, HID, Indala).\n"
    "Hold the card flat against the back of Flipper Zero, about 3-5 cm "
    "from the LF antenna (top of the device).\n"
    "Press OK to start a read session.",

    /* InfoContextSubGhz */
    "Sub-GHz uses the built-in CC1101 transceiver (300-928 MHz).\n"
    "Default frequency is 433.92 MHz.\n"
    "Quick test reads an RSSI sample to verify radio hardware.\n"
    "Check local regulations before transmitting.",

    /* InfoContextHttp */
    "HTTP Client sends requests via the FlipperHTTP board.\n"
    "Supports GET, POST, PUT, DELETE.\n"
    "Maximum URL: 255 chars, body: 1023 chars.\n"
    "Timeout: 10 seconds per request.\n"
    "The response code and first 60 chars of body are shown.",

    /* InfoContextWebSocket */
    "WebSocket Client maintains a persistent connection to a WebSocket "
    "server via the FlipperHTTP board.\n"
    "Press OK to connect/disconnect.\n"
    "Incoming messages update the message count and last message preview.",

    /* InfoContextManual */
    "Tutorial Manual - 13 chapters covering all features.\n"
    "UP/DOWN: scroll chapter list or chapter content\n"
    "OK: open selected chapter\n"
    "BACK: return to chapter list or main menu\n"
    "Long-press OK from any chapter for context help.",

    /* InfoContextSearch */
    "Search the manual by keyword.\n"
    "Use the on-screen keyboard to type a search term.\n"
    "Results show all chapters containing the keyword.\n"
    "Select a result and press OK to jump to that chapter.",
};

InfoOverlay* info_overlay_alloc() {
    InfoOverlay* o = malloc(sizeof(InfoOverlay));
    if(o) {
        memset(o, 0, sizeof(InfoOverlay));
        o->visible = false;
        o->context = InfoContextSplash;
    }
    return o;
}

void info_overlay_free(InfoOverlay* overlay) {
    if(overlay) free(overlay);
}

void info_overlay_show(InfoOverlay* overlay, InfoContext context) {
    if(!overlay) return;
    overlay->visible = true;
    overlay->context = context;
    overlay->scroll_pos = 0;
}

void info_overlay_hide(InfoOverlay* overlay) {
    if(!overlay) return;
    overlay->visible = false;
    overlay->scroll_pos = 0;
}

void info_overlay_scroll_up(InfoOverlay* overlay) {
    if(!overlay || !overlay->visible) return;
    if(overlay->scroll_pos > 0) overlay->scroll_pos--;
}

void info_overlay_scroll_down(InfoOverlay* overlay) {
    if(!overlay || !overlay->visible) return;
    if(overlay->scroll_pos < 255) overlay->scroll_pos++;
}

const char* info_overlay_get_text(InfoContext context) {
    if(context >= InfoContextCount) return "";
    return info_texts[context];
}

void info_overlay_draw(Canvas* canvas, InfoOverlay* overlay) {
    if(!overlay || !overlay->visible) return;

    /* Semi-transparent panel: draw a white filled box with black border */
    canvas_draw_box(canvas, 2, 2, 124, 60);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 2, 2, 124, 60);

    /* Title bar */
    canvas_draw_box(canvas, 2, 2, 124, 12);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 6, 11, "Help");
    canvas_set_color(canvas, ColorBlack);

    /* Content with scrolling */
    const char* text = info_overlay_get_text(overlay->context);
    canvas_set_font(canvas, FontSecondary);

    const int line_h = 9;
    const int max_chars = 20; /* ~120px / 6px per char */
    int cur_line = 0;
    int cur_y = 16;
    char line_buf[32];
    int line_pos = 0;
    const char* p = text;

    while(*p) {
        const char* word_start = p;
        while(*p && *p != ' ' && *p != '\n') p++;
        int word_len = (int)(p - word_start);

        if(line_pos > 0 && line_pos + 1 + word_len > max_chars) {
            if(cur_line >= overlay->scroll_pos) {
                line_buf[line_pos] = '\0';
                canvas_draw_str(canvas, 6, cur_y, line_buf);
                cur_y += line_h;
                if(cur_y > 60) break;
            }
            cur_line++;
            line_pos = 0;
        }
        if(line_pos > 0 && line_pos + 1 < (int)sizeof(line_buf)) line_buf[line_pos++] = ' ';
        if(word_len > 0 && line_pos + word_len < (int)sizeof(line_buf) - 1) {
            memcpy(line_buf + line_pos, word_start, word_len);
            line_pos += word_len;
        }
        if(*p == '\n') {
            if(cur_line >= overlay->scroll_pos) {
                line_buf[line_pos] = '\0';
                canvas_draw_str(canvas, 6, cur_y, line_buf);
                cur_y += line_h;
                if(cur_y > 60) break;
            }
            cur_line++;
            line_pos = 0;
            p++;
        } else if(*p == ' ') {
            p++;
        }
    }
    if(line_pos > 0 && cur_y <= 60 && cur_line >= overlay->scroll_pos) {
        line_buf[line_pos] = '\0';
        canvas_draw_str(canvas, 6, cur_y, line_buf);
    }

    /* "OK to close" hint at bottom */
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 70, 60, "OK=Close");
}
