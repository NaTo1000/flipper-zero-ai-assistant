#pragma once
#include <gui/gui.h>

// Draw dolphin pixel art icons
void ui_draw_dolphin_welcome(Canvas* canvas, int x, int y);
void ui_draw_dolphin_wifi(Canvas* canvas, int x, int y);
void ui_draw_dolphin_wrench(Canvas* canvas, int x, int y);
void ui_draw_dolphin_book(Canvas* canvas, int x, int y);
void ui_draw_dolphin_search(Canvas* canvas, int x, int y);
void ui_draw_dolphin_success(Canvas* canvas, int x, int y);
void ui_draw_dolphin_error(Canvas* canvas, int x, int y);

// Draw small status icons
void ui_draw_info_circle(Canvas* canvas, int x, int y);
void ui_draw_wifi_icon(Canvas* canvas, int x, int y);
void ui_draw_ble_icon(Canvas* canvas, int x, int y);
void ui_draw_nrf_icon(Canvas* canvas, int x, int y);
void ui_draw_lf_icon(Canvas* canvas, int x, int y);
void ui_draw_subghz_icon(Canvas* canvas, int x, int y);
void ui_draw_check_icon(Canvas* canvas, int x, int y);
void ui_draw_cross_icon(Canvas* canvas, int x, int y);
void ui_draw_warning_icon(Canvas* canvas, int x, int y);

// Draw progress bar
void ui_draw_progress_bar(Canvas* canvas, int x, int y, int w, int h, uint8_t percent);

// Draw info button hint
void ui_draw_info_hint(Canvas* canvas);

// Draw header bar
void ui_draw_header(Canvas* canvas, const char* title);

// Draw scrollbar
void ui_draw_scrollbar(Canvas* canvas, int total, int visible, int current);

// Wrap text
void ui_draw_wrapped_text(Canvas* canvas, int x, int y, int max_width, const char* text);
