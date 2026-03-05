#pragma once
#include <furi.h>
#include <gui/gui.h>

typedef enum {
    InfoContextSplash,
    InfoContextDiagnostics,
    InfoContextMainMenu,
    InfoContextWifi,
    InfoContextBle,
    InfoContextNrf,
    InfoContextLf,
    InfoContextSubGhz,
    InfoContextHttp,
    InfoContextWebSocket,
    InfoContextManual,
    InfoContextSearch,
    InfoContextCount
} InfoContext;

typedef struct {
    bool visible;
    InfoContext context;
    uint8_t scroll_pos;
} InfoOverlay;

InfoOverlay* info_overlay_alloc();
void info_overlay_free(InfoOverlay* overlay);
void info_overlay_show(InfoOverlay* overlay, InfoContext context);
void info_overlay_hide(InfoOverlay* overlay);
void info_overlay_draw(Canvas* canvas, InfoOverlay* overlay);
void info_overlay_scroll_up(InfoOverlay* overlay);
void info_overlay_scroll_down(InfoOverlay* overlay);
const char* info_overlay_get_text(InfoContext context);
