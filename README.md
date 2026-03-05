# Flipper Zero — AI Assistant Installer & Tutorial

A full-featured, browser-based **installer wizard and tutorial** for the Flipper Zero AI Assistant firmware.  
Features an **orange Flipper Zero brand theme** and a **live 3D Three.js device preview** built right into the page.

## ✨ Features

- 🔶 **Superior Graphics** — Flipper Zero orange colour palette with animated glow effects, Orbitron display font, and neon-green screen highlights
- 🎲 **Full 3D Rendering** — Interactive Three.js scene with a detailed Flipper Zero model, floating particles, orbiting lights, and mouse parallax
- 🧭 **Step-by-step Installer Wizard** — 6-stage guided installation (requirements → backup → firmware flash → WiFi Dev Board → WiFi config → verify)
- 📚 **Tutorial Library** — Beginner / Intermediate / Advanced tutorial cards covering Sub-GHz, NFC, IR, BadUSB, GPIO, and AI Chat
- ⚡ **Scroll-reveal animations**, FAQ accordion, and one-click code-block copy
- 📱 Fully responsive layout

## 🚀 Quick Start

Open `index.html` in any modern browser — no build step required.

```bash
git clone https://github.com/NaTo1000/flipper-zero-ai-assistant.git
cd flipper-zero-ai-assistant
# open index.html in your browser
```

Or serve locally:

```bash
npx serve .
# then visit http://localhost:3000
```

## 📁 File Structure

| File | Purpose |
|------|---------|
| `index.html` | Main installer & tutorial page |
| `style.css`  | Flipper Zero orange theme, animations |
| `scene3d.js` | Three.js 3D device scene |
| `installer.js` | Stepper wizard, FAQ, scroll-reveal |

## 🛠 Installation Steps (covered in the wizard)

1. **Requirements & Downloads** — hardware checklist, firmware & qFlipper links
2. **Back Up Your Flipper** — SD card backup via device menu or qFlipper
3. **Flash AI Assistant Firmware** — via qFlipper GUI or `ufbt` CLI
4. **Flash WiFi Dev Board** — `esptool.py` commands for ESP32-S2
5. **Configure WiFi & API Key** — captive-portal setup page at `192.168.4.1`
6. **Verify & Start Using** — orange LED pulse confirmation, web UI at `flipperai.local`

## 📜 License

MIT — see [LICENSE](LICENSE).  
Not affiliated with Flipper Devices Inc.
