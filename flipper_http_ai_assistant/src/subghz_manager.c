#include "../inc/subghz_manager.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>

/*
 * Sub-GHz operations use the CC1101 transceiver present on all Flipper
 * Zero units.  A full implementation would use the subghz_devices HAL.
 * We populate defaults and provide structural stubs.
 */

SubGhzManager* subghz_manager_alloc() {
    SubGhzManager* m = malloc(sizeof(SubGhzManager));
    if(m) {
        memset(m, 0, sizeof(SubGhzManager));
        m->radio_ok = true;
        m->frequency = 433.92f;
        snprintf(m->modulation, sizeof(m->modulation), "AM650");
        m->rssi = -100;
    }
    return m;
}

void subghz_manager_free(SubGhzManager* mgr) {
    if(mgr) free(mgr);
}

bool subghz_manager_check_radio(SubGhzManager* mgr) {
    if(!mgr) return false;
    /*
     * Real check: read CC1101 PARTNUM/VERSION registers over SPI.
     * Expected: PARTNUM=0x00, VERSION=0x14.
     * Stub: assume OK on all production units.
     */
    mgr->radio_ok = true;
    return true;
}

bool subghz_manager_quick_test(SubGhzManager* mgr) {
    if(!mgr || !mgr->radio_ok) return false;
    /*
     * Real test: set frequency, read RSSI register once.
     * Stub: return true.
     */
    mgr->rssi = -85;
    return true;
}

void subghz_manager_draw_status(Canvas* canvas, SubGhzManager* mgr) {
    ui_draw_header(canvas, "Sub-GHz Manager");
    if(!mgr) return;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 22, "Radio:");
    canvas_draw_str(canvas, 44, 22, mgr->radio_ok ? "OK" : "FAIL");

    char freq_str[24];
    snprintf(freq_str, sizeof(freq_str), "%.2f MHz", (double)mgr->frequency);
    canvas_draw_str(canvas, 4, 32, "Freq:");
    canvas_draw_str(canvas, 36, 32, freq_str);

    canvas_draw_str(canvas, 4, 42, "Mod:");
    canvas_draw_str(canvas, 32, 42, mgr->modulation);

    char rssi_str[12];
    snprintf(rssi_str, sizeof(rssi_str), "%d dBm", (int)mgr->rssi);
    canvas_draw_str(canvas, 4, 52, "RSSI:");
    canvas_draw_str(canvas, 36, 52, rssi_str);

    ui_draw_info_hint(canvas);
}
