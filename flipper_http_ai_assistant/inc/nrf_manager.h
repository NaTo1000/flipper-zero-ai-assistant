#pragma once
#include <furi.h>

typedef struct {
    bool detected;
    bool configured;
    uint8_t channel;
    uint8_t power;
    char address[6];
} NrfManager;

NrfManager* nrf_manager_alloc();
void nrf_manager_free(NrfManager* mgr);
bool nrf_manager_detect(NrfManager* mgr);
bool nrf_manager_configure(NrfManager* mgr);
bool nrf_manager_test(NrfManager* mgr);
void nrf_manager_draw_status(Canvas* canvas, NrfManager* mgr);
