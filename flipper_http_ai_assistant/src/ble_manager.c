#include "../inc/ble_manager.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <furi_hal_bt.h>
#include <string.h>
#include <stdio.h>

BleManager* ble_manager_alloc() {
    BleManager* m = malloc(sizeof(BleManager));
    if(m) {
        memset(m, 0, sizeof(BleManager));
        snprintf(m->device_name, sizeof(m->device_name), "Flipper Zero");
    }
    return m;
}

void ble_manager_free(BleManager* mgr) {
    if(mgr) free(mgr);
}

bool ble_manager_enable(BleManager* mgr) {
    if(!mgr) return false;
    /* The BLE stack is managed by furi_hal_bt. We signal the radio to
     * start advertising if it is not already active. */
    if(!furi_hal_bt_is_active()) {
        furi_hal_bt_start_advertising();
    }
    mgr->enabled = furi_hal_bt_is_active();
    mgr->advertising = mgr->enabled;
    return mgr->enabled;
}

bool ble_manager_disable(BleManager* mgr) {
    if(!mgr) return false;
    if(furi_hal_bt_is_active()) {
        furi_hal_bt_stop_advertising();
    }
    mgr->enabled = false;
    mgr->advertising = false;
    mgr->connected = false;
    return true;
}

bool ble_manager_get_status(BleManager* mgr) {
    if(!mgr) return false;
    mgr->enabled = furi_hal_bt_is_active();
    /* furi_hal_bt_is_connected is available from FW 0.82 */
    mgr->connected = furi_hal_bt_is_connected();
    return mgr->enabled;
}

void ble_manager_draw_status(Canvas* canvas, BleManager* mgr) {
    ui_draw_header(canvas, "BLE Manager");
    if(!mgr) return;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 22, "Device:");
    canvas_draw_str(canvas, 48, 22, mgr->device_name);

    canvas_draw_str(canvas, 4, 32, "Radio:");
    canvas_draw_str(canvas, 44, 32, mgr->enabled ? "ON" : "OFF");

    canvas_draw_str(canvas, 4, 42, "Advertising:");
    canvas_draw_str(canvas, 76, 42, mgr->advertising ? "Yes" : "No");

    canvas_draw_str(canvas, 4, 52, "Connected:");
    canvas_draw_str(canvas, 68, 52, mgr->connected ? "Yes" : "No");

    ui_draw_info_hint(canvas);
}
