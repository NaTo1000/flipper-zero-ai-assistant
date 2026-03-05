#pragma once
#include <furi.h>

typedef struct {
    bool enabled;
    bool advertising;
    bool connected;
    char device_name[32];
    int8_t rssi;
} BleManager;

BleManager* ble_manager_alloc();
void ble_manager_free(BleManager* mgr);
bool ble_manager_enable(BleManager* mgr);
bool ble_manager_disable(BleManager* mgr);
bool ble_manager_get_status(BleManager* mgr);
void ble_manager_draw_status(Canvas* canvas, BleManager* mgr);
