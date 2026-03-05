#include "../inc/manual.h"
#include "../inc/manual_chapters.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>

ManualState* manual_alloc() {
    ManualState* s = malloc(sizeof(ManualState));
    if(s) {
        memset(s, 0, sizeof(ManualState));
        s->chapter_index = 0;
        s->scroll_pos = 0;
    }
    return s;
}

void manual_free(ManualState* state) {
    if(state) free(state);
}

void manual_scroll_up(ManualState* state) {
    if(!state) return;
    if(state->scroll_pos > 0) state->scroll_pos--;
}

void manual_scroll_down(ManualState* state) {
    if(!state) return;
    if(state->scroll_pos < 255) state->scroll_pos++;
}

/* Case-insensitive substring search helper */
static char char_to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
}

/* Case-insensitive substring search */
static bool str_icontains(const char* haystack, const char* needle) {
    if(!haystack || !needle || !*needle) return false;
    size_t nlen = strlen(needle);
    for(; *haystack; haystack++) {
        bool match = true;
        for(size_t i = 0; i < nlen; i++) {
            if(char_to_lower(haystack[i]) != char_to_lower(needle[i])) {
                match = false;
                break;
            }
        }
        if(match) return true;
    }
    return false;
}

void manual_search(ManualState* state, const char* query) {
    if(!state || !query) return;
    snprintf(state->search_query, sizeof(state->search_query), "%s", query);
    state->search_result_count = 0;

    for(uint8_t i = 0; i < MANUAL_CHAPTER_COUNT; i++) {
        if(str_icontains(manual_get_chapter_title(i), query) ||
           str_icontains(manual_get_chapter_content(i), query)) {
            state->search_results[state->search_result_count++] = (int8_t)i;
        }
    }
    state->searching = true;
}

/* Draw Table of Contents */
void manual_draw_toc(Canvas* canvas, ManualState* state) {
    ui_draw_header(canvas, "Manual");
    if(!state) return;

    canvas_set_font(canvas, FontSecondary);
    /* Show up to 5 titles fitting in the 64-12=52 px area below header */
    uint8_t visible = 5;
    for(uint8_t i = 0; i < visible; i++) {
        uint8_t idx = state->scroll_pos + i;
        if(idx >= MANUAL_CHAPTER_COUNT) break;
        int y = 14 + i * 10;

        /* Highlight selected chapter */
        if(idx == state->chapter_index) {
            canvas_draw_box(canvas, 0, y - 8, 122, 10);
            canvas_set_color(canvas, ColorWhite);
        }
        char line[30];
        snprintf(line, sizeof(line), "%.29s", manual_get_chapter_title(idx));
        canvas_draw_str(canvas, 2, y, line);
        canvas_set_color(canvas, ColorBlack);
    }
    ui_draw_scrollbar(canvas, MANUAL_CHAPTER_COUNT, visible, state->scroll_pos);
    ui_draw_info_hint(canvas);
}

/* Draw chapter content with scrolling */
void manual_draw_chapter(Canvas* canvas, ManualState* state) {
    if(!state) return;

    const char* title = manual_get_chapter_title(state->chapter_index);
    ui_draw_header(canvas, title);

    const char* content = manual_get_chapter_content(state->chapter_index);
    if(!content) return;

    canvas_set_font(canvas, FontSecondary);
    /* Skip scroll_pos lines of the content */
    const int line_h = 10;
    const int max_width = 120;
    const int char_w = 6;
    int max_chars = max_width / char_w;
    if(max_chars < 1) max_chars = 1;

    int cur_line = 0;
    int cur_y = 14;
    char line_buf[64];
    int line_pos = 0;
    const char* p = content;

    while(*p) {
        const char* word_start = p;
        while(*p && *p != ' ' && *p != '\n') p++;
        int word_len = (int)(p - word_start);

        if(line_pos > 0 && line_pos + 1 + word_len > max_chars) {
            /* Flush line */
            if(cur_line >= state->scroll_pos) {
                line_buf[line_pos] = '\0';
                canvas_draw_str(canvas, 4, cur_y, line_buf);
                cur_y += line_h;
                if(cur_y > 62) break;
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
            if(cur_line >= state->scroll_pos) {
                line_buf[line_pos] = '\0';
                canvas_draw_str(canvas, 4, cur_y, line_buf);
                cur_y += line_h;
                if(cur_y > 62) break;
            }
            cur_line++;
            line_pos = 0;
            p++;
        } else if(*p == ' ') {
            p++;
        }
    }
    /* Flush remaining */
    if(line_pos > 0 && cur_y <= 62 && cur_line >= state->scroll_pos) {
        line_buf[line_pos] = '\0';
        canvas_draw_str(canvas, 4, cur_y, line_buf);
    }
}

/* Draw search results */
void manual_draw_search(Canvas* canvas, ManualState* state) {
    ui_draw_header(canvas, "Search Results");
    if(!state) return;

    canvas_set_font(canvas, FontSecondary);

    if(state->search_result_count == 0) {
        canvas_draw_str(canvas, 4, 28, "No results found");
        canvas_draw_str(canvas, 4, 38, state->search_query);
        return;
    }

    uint8_t visible = 5;
    for(uint8_t i = 0; i < visible && i < state->search_result_count; i++) {
        int8_t idx = state->search_results[i];
        int y = 14 + i * 10;
        char line[30];
        snprintf(line, sizeof(line), "%.29s", manual_get_chapter_title((uint8_t)idx));
        canvas_draw_str(canvas, 2, y, line);
    }
    ui_draw_info_hint(canvas);
}
