#include "../inc/manual_chapters.h"
#include <stddef.h>

/* 13 chapters covering all major aspects of FlipperHTTP AI Assistant */

const char* manual_chapter_titles[MANUAL_CHAPTER_COUNT] = {
    "1. Introduction",
    "2. Hardware Setup",
    "3. WiFi Configuration",
    "4. FlipperHTTP Protocol",
    "5. HTTP Client Usage",
    "6. WebSocket Client",
    "7. JSON Parsing",
    "8. BLE Manager",
    "9. nRF24 Module",
    "10. LF RFID Reader",
    "11. Sub-GHz Radio",
    "12. Diagnostics",
    "13. Troubleshooting",
};

const char* manual_chapter_content[MANUAL_CHAPTER_COUNT] = {
    /* 1. Introduction */
    "FlipperHTTP AI Assistant connects your Flipper Zero to WiFi via "
    "the FlipperHTTP ESP32 add-on board. It provides HTTP requests, "
    "WebSocket connections, JSON parsing, and an AI-guided setup wizard. "
    "Use the D-pad to navigate menus. Press BACK to go up one level. "
    "Hold OK on any screen to show context-sensitive help.",

    /* 2. Hardware Setup */
    "Required hardware:\n"
    "- Flipper Zero (any firmware 0.79+)\n"
    "- FlipperHTTP ESP32 board (GPIO header)\n"
    "Connect the FlipperHTTP board to the Flipper GPIO header. "
    "Pin 13 = USART TX, Pin 14 = USART RX, Pin 9 = 3.3V, Pin 11 = GND. "
    "Power cycle the Flipper after attaching the board. "
    "Run Diagnostics from the main menu to verify detection.",

    /* 3. WiFi Configuration */
    "To configure WiFi:\n"
    "1. Open WiFi Manager from the main menu.\n"
    "2. Select 'Scan' to find nearby networks.\n"
    "3. Select your network from the list.\n"
    "4. Enter the password using the on-screen keyboard.\n"
    "5. Press OK to connect.\n"
    "Credentials are saved to /ext/apps/GPIO/flipperhttp_wifi.cfg "
    "and loaded automatically on next launch.",

    /* 4. FlipperHTTP Protocol */
    "The FlipperHTTP board communicates over USART at 115200 baud. "
    "Commands are ASCII strings with square-bracket tags:\n"
    "[PING] -> [PONG]\n"
    "[VERSION] -> [VERSION/RESULT]{...}\n"
    "[WIFI/CONNECT]{...} -> [WIFI/CONNECTED]\n"
    "[WIFI/SCAN] -> [WIFI/SCAN/RESULT]{...} x N, [WIFI/SCAN/END]\n"
    "[HTTP/GET]{...} -> [HTTP/RESPONSE]{...}\n"
    "[WS/CONNECT]{...} -> [WS/CONNECTED]\n"
    "All commands end with \\r\\n.",

    /* 5. HTTP Client Usage */
    "The HTTP client sends requests via the FlipperHTTP board:\n"
    "GET: [HTTP/GET]{\"url\":\"http://...\",\"headers\":\"...\"}\n"
    "POST: [HTTP/POST]{\"url\":\"...\",\"body\":\"...\"}\n"
    "PUT: [HTTP/PUT]{\"url\":\"...\",\"body\":\"...\"}\n"
    "DELETE: [HTTP/DELETE]{\"url\":\"...\"}\n"
    "Response: [HTTP/RESPONSE]{\"code\":200,\"body\":\"...\"}\n"
    "Maximum URL length: 255 chars. Maximum body: 1023 chars. "
    "Timeout: 10 seconds per request.",

    /* 6. WebSocket Client */
    "WebSocket connections persist until explicitly closed:\n"
    "Connect: [WS/CONNECT]{\"url\":\"wss://...\"}\n"
    "Messages arrive as: [WS/MESSAGE]{\"data\":\"...\"}\n"
    "Disconnect: [WS/DISCONNECT]\n"
    "The WebSocket screen shows connection state, URL, message count, "
    "and a preview of the last received message. "
    "Press OK to connect/disconnect. Press BACK to return to menu.",

    /* 7. JSON Parsing */
    "The built-in JSON parser handles flat key-value objects. "
    "It is used internally by all HTTP/WebSocket responses. "
    "Supported types: strings (\"key\":\"value\"), numbers (\"key\":42), "
    "booleans (\"key\":true), and arrays (\"key\":[...]). "
    "Nested objects are not yet supported. "
    "Call json_parser_get_value() to extract a field by name.",

    /* 8. BLE Manager */
    "The BLE manager controls Flipper Zero's built-in Bluetooth. "
    "Enable: starts advertising as 'Flipper Zero'. "
    "Disable: stops the BLE radio. "
    "Note: BLE and USART share the STM32 co-processor. "
    "Heavy BLE traffic may slow down serial communication. "
    "The BLE manager uses furi_hal_bt_* HAL functions.",

    /* 9. nRF24 Module */
    "The nRF24L01+ module connects via SPI on the GPIO header. "
    "Default channel: 76 (2.476 GHz). "
    "Default address: 0xE7E7E7E7E7. "
    "Detection reads the CONFIG register (expected 0x08). "
    "Note: An nRF24 add-on board is required. "
    "The module is NOT built into Flipper Zero. "
    "Status shows 'Not detected' when no board is connected.",

    /* 10. LF RFID Reader */
    "The built-in 125 kHz LF antenna reads EM4100, HID, and Indala "
    "cards and fobs. Press OK on the LF screen to start reading. "
    "Hold the card flat against the back of Flipper Zero. "
    "Successful reads show the tag ID in hexadecimal. "
    "The tag type is auto-detected from the data format. "
    "Reading range: approximately 3-5 cm.",

    /* 11. Sub-GHz Radio */
    "The CC1101 transceiver operates from 300-928 MHz. "
    "Default frequency: 433.92 MHz (EU/US garage bands). "
    "Modulations: AM650, AM270, FM238, FM476. "
    "RSSI shows the current noise floor. "
    "Quick test reads one RSSI sample to verify the radio. "
    "Note: Transmitting on licensed frequencies may require a license "
    "in your jurisdiction.",

    /* 12. Diagnostics */
    "Diagnostics probes all hardware modules on startup:\n"
    "WiFi: PING/PONG over USART (FlipperHTTP board required)\n"
    "BLE: furi_hal_bt_is_active() check\n"
    "nRF24: SPI CONFIG register read (add-on required)\n"
    "LF: Antenna present on all production units\n"
    "Sub-GHz: CC1101 present on all production units\n"
    "Status icons: [OK] = working, [--] = not found, [!!] = error, "
    "[??] = not checked.",

    /* 13. Troubleshooting */
    "Common issues:\n"
    "- 'WiFi not found': Check FlipperHTTP board connection and power.\n"
    "- 'Connect failed': Verify SSID/password. Check 2.4 GHz vs 5 GHz.\n"
    "- 'No internet': Check router/firewall settings.\n"
    "- 'HTTP timeout': Increase timeout or check WiFi signal strength.\n"
    "- 'BLE not active': Restart Flipper Zero.\n"
    "- App crashes: Ensure firmware 0.79+ is installed.\n"
    "For support visit: github.com/jblanked/FlipperHTTP",
};

const char* manual_get_chapter_title(uint8_t index) {
    if(index >= MANUAL_CHAPTER_COUNT) return "";
    return manual_chapter_titles[index];
}

const char* manual_get_chapter_content(uint8_t index) {
    if(index >= MANUAL_CHAPTER_COUNT) return "";
    return manual_chapter_content[index];
}
