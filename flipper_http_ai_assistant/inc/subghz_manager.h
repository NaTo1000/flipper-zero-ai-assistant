#pragma once
#include <furi.h>

typedef struct {
    bool radio_ok;
    float frequency;
    char modulation[16];
    int8_t rssi;
} SubGhzManager;

SubGhzManager* subghz_manager_alloc();
void subghz_manager_free(SubGhzManager* mgr);
bool subghz_manager_check_radio(SubGhzManager* mgr);
bool subghz_manager_quick_test(SubGhzManager* mgr);
void subghz_manager_draw_status(Canvas* canvas, SubGhzManager* mgr);
