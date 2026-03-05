# FlipperHTTP AI Assistant

An autonomous AI-powered installation, diagnostics, connection management, and tutorial application for the Flipper Zero. Built as a native C `.FAP` external app targeting the official and Momentum firmwares.

---

## Features

- **Startup Diagnostics** — Automatically detects and pings all connected hardware modules (WiFi/ESP32, BLE, NRF24, LF antenna, Sub-GHz CC1101) and generates a status report on launch.
- **ⓘ Context Help** — Long-press OK on any screen to reveal context-sensitive help explaining the current screen, connected hardware, and troubleshooting tips.
- **WiFi Manager** — Scan networks, enter credentials, connect/disconnect, get IP address, and save credentials to SD card (`/ext/apps/GPIO/flipperhttp_wifi.cfg`).
- **AI Auto-Setup** — Autonomous installation agent that guides the user through WiFi configuration and verifies connectivity step-by-step.
- **HTTP Client** — Send GET, POST, PUT, and DELETE requests over WiFi via the FlipperHTTP ESP32 board.
- **WebSocket Client** — Establish persistent WebSocket connections and display live incoming messages.
- **JSON Parser** — Parse flat JSON API responses and extract individual fields (used by HTTP/WebSocket internally).
- **BLE Manager** — Enable/disable the Flipper Zero's built-in Bluetooth Low Energy radio.
- **NRF24 Module** — Detect, configure, and test an optional NRF24L01+ SPI add-on module.
- **LF RFID** — Read 125 kHz RFID tags via the built-in low-frequency antenna.
- **Sub-GHz Radio** — Check CC1101 radio status and RSSI at 433.92 MHz.
- **13-Chapter Tutorial Manual** — Illustrated in-app manual with a keyword search index covering every feature.
- **Dolphin-themed icons** — 16 custom 1-bit pixel-art PNG assets compiled by the Flipper SDK.

---

## Hardware Requirements

| Component | Required |
|---|---|
| Flipper Zero (any colour) | ✅ Yes |
| Firmware 0.79+ (official or Momentum) | ✅ Yes |
| FlipperHTTP ESP32 add-on board | ✅ For WiFi/HTTP/WebSocket features |
| NRF24L01+ SPI add-on board | Optional |

### FlipperHTTP Board Pin-Out

| Flipper GPIO Pin | ESP32 Signal |
|---|---|
| Pin 13 (USART TX) | RX |
| Pin 14 (USART RX) | TX |
| Pin 9 (3.3 V) | VCC |
| Pin 11 (GND) | GND |

Communicate at **115200 baud** using the FlipperHTTP ASCII command protocol.

---

## Building

### Using `ufbt` (recommended)

```bash
# Install ufbt if not already installed
pip install ufbt

# Build from the flipper_http_ai_assistant/ directory
cd flipper_http_ai_assistant
ufbt build
```

The compiled `.fap` will appear at `.ufbt/build/flipper_http_ai_assistant.fap`.

### Using `fbt` (full Flipper SDK)

```bash
# Clone the Flipper Zero firmware
git clone --recursive https://github.com/flipperdevices/flipperzero-firmware
cd flipperzero-firmware

# Copy the app into applications_user
cp -r /path/to/flipper_http_ai_assistant applications_user/

# Build
./fbt fap_flipper_http_ai_assistant
```

### Deploying to Device

```bash
# With ufbt (auto-detects connected Flipper)
ufbt launch

# Or copy manually
cp .ufbt/build/flipper_http_ai_assistant.fap /path/to/flipper/apps/GPIO/
```

---

## Project Structure

```
flipper_http_ai_assistant/
├── application.fam                      ← FAP manifest (GPIO category, v1.0)
├── flipper_http_ai_assistant.c          ← Main entry point & scene manager
├── src/
│   ├── diagnostics.c                    ← Startup diagnostics engine
│   ├── wifi_manager.c                   ← WiFi connection management
│   ├── ble_manager.c                    ← BLE status and control
│   ├── nrf_manager.c                    ← NRF24 detection and config
│   ├── lf_manager.c                     ← LF RFID antenna management
│   ├── subghz_manager.c                 ← Sub-GHz radio management
│   ├── http_client.c                    ← HTTP GET/POST/PUT/DELETE
│   ├── websocket_client.c               ← WebSocket connection manager
│   ├── json_parser.c                    ← JSON parsing utilities
│   ├── ai_agent.c                       ← Autonomous AI installation agent
│   ├── manual.c                         ← In-app manual system
│   ├── manual_chapters.c                ← Chapter content (13 chapters)
│   ├── search_index.c                   ← Keyword search/index
│   ├── info_overlay.c                   ← ⓘ context-help overlay
│   └── ui_helpers.c                     ← UI helpers and dolphin pixel art
├── inc/                                 ← Header files (one per module)
└── assets/icons/                        ← 1-bit PNG icons (16 files)
    ├── dolphin_welcome_48x48.png
    ├── dolphin_wifi_48x48.png
    ├── dolphin_wrench_48x48.png
    ├── dolphin_book_48x48.png
    ├── dolphin_search_48x48.png
    ├── dolphin_success_48x48.png
    ├── dolphin_error_48x48.png
    ├── info_circle_10x10.png
    ├── wifi_icon_12x12.png
    ├── ble_icon_12x12.png
    ├── nrf_icon_12x12.png
    ├── lf_icon_12x12.png
    ├── subghz_icon_12x12.png
    ├── check_icon_8x8.png
    ├── cross_icon_8x8.png
    └── warning_icon_8x8.png
```

---

## FlipperHTTP Serial Protocol

The app communicates with the ESP32 board over USART at 115200 baud using ASCII commands:

| Command | Description | Response |
|---|---|---|
| `[PING]\r\n` | Check board present | `[PONG]` |
| `[VERSION]\r\n` | Get firmware version | `[VERSION/RESULT]{...}` |
| `[WIFI/SCAN]\r\n` | Scan networks | `[WIFI/SCAN/RESULT]{...}` × N, `[WIFI/SCAN/END]` |
| `[WIFI/CONNECT]{...}\r\n` | Connect to network | `[WIFI/CONNECTED]` or `[WIFI/ERROR]` |
| `[WIFI/DISCONNECT]\r\n` | Disconnect | `[WIFI/DISCONNECTED]` |
| `[WIFI/IP]\r\n` | Get IP address | `[IP/ADDRESS]{...}` |
| `[HTTP/GET]{...}\r\n` | HTTP GET request | `[HTTP/RESPONSE]{...}` |
| `[HTTP/POST]{...}\r\n` | HTTP POST request | `[HTTP/RESPONSE]{...}` |
| `[HTTP/PUT]{...}\r\n` | HTTP PUT request | `[HTTP/RESPONSE]{...}` |
| `[HTTP/DELETE]{...}\r\n` | HTTP DELETE request | `[HTTP/RESPONSE]{...}` |
| `[WS/CONNECT]{...}\r\n` | Open WebSocket | `[WS/CONNECTED]` |
| `[WS/DISCONNECT]\r\n` | Close WebSocket | `[WS/DISCONNECTED]` |

Based on the enhanced FlipperHTTP JavaScript examples from [jclaudan/flipper-zero-tutorials](https://github.com/jclaudan/flipper-zero-tutorials/commit/06022ad1d4622904d4705bb1005c2f10e29e1950).

---

## Tutorial Manual Chapters

| # | Chapter |
|---|---|
| 1 | Introduction |
| 2 | Hardware Setup |
| 3 | WiFi Configuration |
| 4 | FlipperHTTP Protocol |
| 5 | HTTP Client Usage |
| 6 | WebSocket Client |
| 7 | JSON Parsing |
| 8 | BLE Manager |
| 9 | NRF24 Module |
| 10 | LF RFID Reader |
| 11 | Sub-GHz Radio |
| 12 | Diagnostics |
| 13 | Troubleshooting |

Navigate with Up/Down arrows. Press OK to open a chapter. Long-press OK for context help. Press Back to return to the Table of Contents.

---

## Navigation

| Button | Action |
|---|---|
| Up / Down | Move selection |
| Left / Right | Scroll / adjust |
| OK (short press) | Confirm / select |
| OK (long press) | Show ⓘ context help |
| Back | Go back / exit |

---

## License

MIT — see [LICENSE](LICENSE).

## Credits

- **JBlanked** — [FlipperHTTP](https://github.com/jblanked/FlipperHTTP) serial protocol and ESP32 firmware
- **jamisonderek** — Original [flipper-zero-tutorials](https://github.com/jamisonderek/flipper-zero-tutorials) foundation
- **jclaudan** — Enhanced FlipperHTTP JavaScript examples (WiFi, HTTP, WebSocket, binary data, catfact API)
