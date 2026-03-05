#pragma once
#include <furi.h>

typedef struct {
    bool antenna_ok;
    bool reading;
    uint32_t last_tag;
    char tag_type[32];
} LfManager;

LfManager* lf_manager_alloc();
void lf_manager_free(LfManager* mgr);
bool lf_manager_check_antenna(LfManager* mgr);
bool lf_manager_read_tag(LfManager* mgr);
void lf_manager_draw_status(Canvas* canvas, LfManager* mgr);
