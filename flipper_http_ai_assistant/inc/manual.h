#pragma once
#include <furi.h>
#include <gui/gui.h>

#define MANUAL_CHAPTER_COUNT 13

typedef struct {
    uint8_t chapter_index;
    uint8_t scroll_pos;
    char search_query[32];
    bool searching;
    int8_t search_results[MANUAL_CHAPTER_COUNT];
    uint8_t search_result_count;
} ManualState;

ManualState* manual_alloc();
void manual_free(ManualState* state);
void manual_draw_toc(Canvas* canvas, ManualState* state);
void manual_draw_chapter(Canvas* canvas, ManualState* state);
void manual_draw_search(Canvas* canvas, ManualState* state);
void manual_scroll_up(ManualState* state);
void manual_scroll_down(ManualState* state);
void manual_search(ManualState* state, const char* query);
