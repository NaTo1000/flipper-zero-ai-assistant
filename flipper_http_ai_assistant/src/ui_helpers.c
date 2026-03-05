#include "../inc/ui_helpers.h"
#include <gui/gui.h>
#include <string.h>
#include <stdio.h>

/* ── Dolphin pixel-art helpers ──────────────────────────────────────────── */

/*
 * Every dolphin is drawn as a compact 1-bit pixel shape using
 * canvas_draw_dot and canvas_draw_line primitives so no external
 * icon assets are needed at runtime.  The coordinate origin (x,y)
 * is the top-left corner of the 48×48 bounding box.
 */

/* Draw a filled ellipse approximation using horizontal lines */
static void draw_filled_ellipse(Canvas* canvas, int cx, int cy, int rx, int ry) {
    for(int dy = -ry; dy <= ry; dy++) {
        float ratio = 1.0f - (float)(dy * dy) / (float)(ry * ry);
        if(ratio < 0.0f) ratio = 0.0f;
        int half = (int)((float)rx * __builtin_sqrt((double)ratio));
        canvas_draw_line(canvas, cx - half, cy + dy, cx + half, cy + dy);
    }
}

void ui_draw_dolphin_welcome(Canvas* canvas, int x, int y) {
    /* Body arc – roughly 30×14 oval */
    draw_filled_ellipse(canvas, x + 24, y + 28, 14, 7);
    /* Head */
    draw_filled_ellipse(canvas, x + 34, y + 24, 7, 5);
    /* Beak / rostrum */
    canvas_draw_line(canvas, x + 40, y + 25, x + 47, y + 23);
    canvas_draw_line(canvas, x + 40, y + 26, x + 47, y + 25);
    /* Dorsal fin */
    canvas_draw_line(canvas, x + 26, y + 21, x + 30, y + 14);
    canvas_draw_line(canvas, x + 30, y + 14, x + 34, y + 21);
    /* Tail flukes */
    canvas_draw_line(canvas, x + 10, y + 26, x + 4, y + 20);
    canvas_draw_line(canvas, x + 4, y + 20, x + 8, y + 28);
    canvas_draw_line(canvas, x + 10, y + 30, x + 4, y + 36);
    canvas_draw_line(canvas, x + 4, y + 36, x + 8, y + 30);
    /* Waving flipper */
    canvas_draw_line(canvas, x + 28, y + 32, x + 22, y + 38);
    canvas_draw_line(canvas, x + 22, y + 38, x + 30, y + 37);
    /* Eye */
    canvas_draw_dot(canvas, x + 38, y + 22);
}

void ui_draw_dolphin_wifi(Canvas* canvas, int x, int y) {
    ui_draw_dolphin_welcome(canvas, x, y);
    /* WiFi arcs above dolphin */
    canvas_draw_frame(canvas, x + 18, y + 4, 12, 6);
    canvas_draw_frame(canvas, x + 21, y + 6, 6, 4);
    canvas_draw_dot(canvas, x + 23, y + 8);
    canvas_draw_dot(canvas, x + 24, y + 8);
}

void ui_draw_dolphin_wrench(Canvas* canvas, int x, int y) {
    ui_draw_dolphin_welcome(canvas, x, y);
    /* Wrench shape */
    canvas_draw_line(canvas, x + 10, y + 8, x + 18, y + 16);
    canvas_draw_frame(canvas, x + 8, y + 6, 6, 4);
    canvas_draw_line(canvas, x + 18, y + 16, x + 14, y + 20);
}

void ui_draw_dolphin_book(Canvas* canvas, int x, int y) {
    ui_draw_dolphin_welcome(canvas, x, y);
    /* Open book */
    canvas_draw_frame(canvas, x + 8, y + 6, 10, 12);
    canvas_draw_frame(canvas, x + 18, y + 6, 10, 12);
    canvas_draw_line(canvas, x + 18, y + 6, x + 18, y + 18);
    canvas_draw_line(canvas, x + 10, y + 10, x + 16, y + 10);
    canvas_draw_line(canvas, x + 10, y + 13, x + 16, y + 13);
    canvas_draw_line(canvas, x + 20, y + 10, x + 26, y + 10);
    canvas_draw_line(canvas, x + 20, y + 13, x + 26, y + 13);
}

void ui_draw_dolphin_search(Canvas* canvas, int x, int y) {
    ui_draw_dolphin_welcome(canvas, x, y);
    /* Magnifying glass */
    canvas_draw_frame(canvas, x + 8, y + 4, 10, 10);
    canvas_draw_line(canvas, x + 18, y + 14, x + 22, y + 18);
}

void ui_draw_dolphin_success(Canvas* canvas, int x, int y) {
    ui_draw_dolphin_welcome(canvas, x, y);
    /* Checkmark */
    canvas_draw_line(canvas, x + 8, y + 10, x + 12, y + 14);
    canvas_draw_line(canvas, x + 12, y + 14, x + 20, y + 6);
}

void ui_draw_dolphin_error(Canvas* canvas, int x, int y) {
    ui_draw_dolphin_welcome(canvas, x, y);
    /* X mark */
    canvas_draw_line(canvas, x + 8, y + 6, x + 18, y + 16);
    canvas_draw_line(canvas, x + 18, y + 6, x + 8, y + 16);
}

/* ── Small status icons ─────────────────────────────────────────────────── */

void ui_draw_info_circle(Canvas* canvas, int x, int y) {
    /* 10×10 circle with "i" */
    canvas_draw_frame(canvas, x, y, 10, 10);
    canvas_draw_dot(canvas, x + 5, y + 2);
    canvas_draw_line(canvas, x + 5, y + 4, x + 5, y + 8);
}

void ui_draw_wifi_icon(Canvas* canvas, int x, int y) {
    /* 12×12 WiFi symbol – three arcs + dot */
    canvas_draw_line(canvas, x + 1, y + 8, x + 3, y + 6);
    canvas_draw_line(canvas, x + 9, y + 8, x + 11, y + 6);
    canvas_draw_line(canvas, x + 3, y + 6, x + 9, y + 6);
    canvas_draw_line(canvas, x + 2, y + 5, x + 4, y + 3);
    canvas_draw_line(canvas, x + 8, y + 5, x + 10, y + 3);
    canvas_draw_line(canvas, x + 4, y + 3, x + 8, y + 3);
    canvas_draw_line(canvas, x + 0, y + 3, x + 2, y + 1);
    canvas_draw_line(canvas, x + 10, y + 3, x + 12, y + 1);
    canvas_draw_line(canvas, x + 2, y + 1, x + 10, y + 1);
    canvas_draw_dot(canvas, x + 6, y + 10);
    canvas_draw_dot(canvas, x + 5, y + 10);
}

void ui_draw_ble_icon(Canvas* canvas, int x, int y) {
    /* 12×12 Bluetooth symbol */
    canvas_draw_line(canvas, x + 6, y + 1, x + 6, y + 11);
    canvas_draw_line(canvas, x + 6, y + 1, x + 10, y + 4);
    canvas_draw_line(canvas, x + 10, y + 4, x + 6, y + 7);
    canvas_draw_line(canvas, x + 6, y + 7, x + 10, y + 10);
    canvas_draw_line(canvas, x + 10, y + 10, x + 6, y + 11);
    canvas_draw_line(canvas, x + 6, y + 1, x + 2, y + 4);
    canvas_draw_line(canvas, x + 2, y + 10, x + 6, y + 7);
}

void ui_draw_nrf_icon(Canvas* canvas, int x, int y) {
    /* 12×12 RF symbol */
    canvas_draw_line(canvas, x + 6, y + 6, x + 6, y + 11);
    canvas_draw_line(canvas, x + 4, y + 4, x + 8, y + 4);
    canvas_draw_line(canvas, x + 2, y + 2, x + 10, y + 2);
    canvas_draw_line(canvas, x + 0, y + 0, x + 12, y + 0);
    canvas_draw_dot(canvas, x + 6, y + 6);
}

void ui_draw_lf_icon(Canvas* canvas, int x, int y) {
    /* 12×12 LF wave */
    canvas_draw_line(canvas, x + 0, y + 6, x + 3, y + 3);
    canvas_draw_line(canvas, x + 3, y + 3, x + 6, y + 6);
    canvas_draw_line(canvas, x + 6, y + 6, x + 9, y + 9);
    canvas_draw_line(canvas, x + 9, y + 9, x + 12, y + 6);
    canvas_draw_str(canvas, x + 2, y + 12, "LF");
}

void ui_draw_subghz_icon(Canvas* canvas, int x, int y) {
    /* 12×12 radio wave */
    canvas_draw_line(canvas, x + 6, y + 8, x + 6, y + 12);
    canvas_draw_line(canvas, x + 3, y + 6, x + 9, y + 6);
    canvas_draw_line(canvas, x + 1, y + 3, x + 11, y + 3);
    canvas_draw_dot(canvas, x + 6, y + 9);
}

void ui_draw_check_icon(Canvas* canvas, int x, int y) {
    /* 8×8 checkmark */
    canvas_draw_line(canvas, x + 1, y + 4, x + 3, y + 6);
    canvas_draw_line(canvas, x + 3, y + 6, x + 7, y + 2);
}

void ui_draw_cross_icon(Canvas* canvas, int x, int y) {
    /* 8×8 X */
    canvas_draw_line(canvas, x + 1, y + 1, x + 7, y + 7);
    canvas_draw_line(canvas, x + 7, y + 1, x + 1, y + 7);
}

void ui_draw_warning_icon(Canvas* canvas, int x, int y) {
    /* 8×8 triangle with ! */
    canvas_draw_line(canvas, x + 4, y + 0, x + 0, y + 7);
    canvas_draw_line(canvas, x + 0, y + 7, x + 8, y + 7);
    canvas_draw_line(canvas, x + 8, y + 7, x + 4, y + 0);
    canvas_draw_dot(canvas, x + 4, y + 3);
    canvas_draw_dot(canvas, x + 4, y + 5);
}

/* ── Compound UI widgets ────────────────────────────────────────────────── */

void ui_draw_progress_bar(Canvas* canvas, int x, int y, int w, int h, uint8_t percent) {
    canvas_draw_frame(canvas, x, y, w, h);
    int fill = (w - 2) * percent / 100;
    if(fill > 0) {
        canvas_draw_box(canvas, x + 1, y + 1, fill, h - 2);
    }
}

void ui_draw_info_hint(Canvas* canvas) {
    /* Small "Hold OK = Info" hint in the bottom-right corner */
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 72, 63, "[OK] Info");
}

void ui_draw_header(Canvas* canvas, const char* title) {
    canvas_draw_box(canvas, 0, 0, 128, 12);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 4, 10, title);
    canvas_set_color(canvas, ColorBlack);
}

void ui_draw_scrollbar(Canvas* canvas, int total, int visible, int current) {
    if(total <= visible) return;
    int bar_h = 64 * visible / total;
    int bar_y = 64 * current / total;
    canvas_draw_frame(canvas, 124, 0, 4, 64);
    canvas_draw_box(canvas, 125, bar_y, 2, bar_h);
}

void ui_draw_wrapped_text(Canvas* canvas, int x, int y, int max_width, const char* text) {
    if(!text) return;
    canvas_set_font(canvas, FontSecondary);

    /* Very simple word-wrap: split on spaces, track line width.
     * The Flipper SDK doesn't expose text measurement so we use
     * an approximation of 6 pixels per character for FontSecondary. */
    const int char_w = 6;
    const int line_h = 10;
    int max_chars = max_width / char_w;
    if(max_chars < 1) max_chars = 1;

    char line_buf[64];
    int line_pos = 0;
    int cur_y = y;
    const char* p = text;

    while(*p) {
        /* Find next word end */
        const char* word_start = p;
        while(*p && *p != ' ' && *p != '\n') p++;
        int word_len = (int)(p - word_start);

        if(line_pos > 0 && line_pos + 1 + word_len > max_chars) {
            line_buf[line_pos] = '\0';
            canvas_draw_str(canvas, x, cur_y, line_buf);
            cur_y += line_h;
            line_pos = 0;
            if(cur_y > 64) break;
        }
        if(line_pos > 0 && line_pos + 1 < (int)sizeof(line_buf)) {
            line_buf[line_pos++] = ' ';
        }
        if(word_len > 0 && line_pos + word_len < (int)sizeof(line_buf) - 1) {
            memcpy(line_buf + line_pos, word_start, word_len);
            line_pos += word_len;
        }
        if(*p == '\n') {
            line_buf[line_pos] = '\0';
            canvas_draw_str(canvas, x, cur_y, line_buf);
            cur_y += line_h;
            line_pos = 0;
            p++;
            if(cur_y > 64) break;
        } else if(*p == ' ') {
            p++;
        }
    }
    /* Flush remaining */
    if(line_pos > 0 && cur_y <= 64) {
        line_buf[line_pos] = '\0';
        canvas_draw_str(canvas, x, cur_y, line_buf);
    }
}
