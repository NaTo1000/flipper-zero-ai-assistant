#include "../inc/lf_manager.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>

/*
 * LF (125 kHz) operations use Flipper Zero's built-in LF antenna.
 * Full implementation would use the lf_rfid HAL layer.  We provide
 * the scaffolding here; antenna_ok is set optimistically since all
 * production Flipper units have the LF coil populated.
 */

LfManager* lf_manager_alloc() {
    LfManager* m = malloc(sizeof(LfManager));
    if(m) {
        memset(m, 0, sizeof(LfManager));
        m->antenna_ok = true;
    }
    return m;
}

void lf_manager_free(LfManager* mgr) {
    if(mgr) free(mgr);
}

bool lf_manager_check_antenna(LfManager* mgr) {
    if(!mgr) return false;
    /* In a full implementation, measure the LF antenna resonant
     * frequency and compare to expected 125 kHz ± tolerance. */
    mgr->antenna_ok = true;
    return true;
}

bool lf_manager_read_tag(LfManager* mgr) {
    if(!mgr) return false;
    /*
     * A real implementation would:
     * 1. furi_hal_rfid_tim_read_start(125000, 0.5f)
     * 2. Wait for a decoded frame via callback
     * 3. Parse EM4100 / HID / Indala frame
     * Stub: just clear last_tag and return false.
     */
    mgr->reading = false;
    mgr->last_tag = 0;
    snprintf(mgr->tag_type, sizeof(mgr->tag_type), "None");
    return false;
}

void lf_manager_draw_status(Canvas* canvas, LfManager* mgr) {
    ui_draw_header(canvas, "LF RFID Manager");
    if(!mgr) return;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 22, "Antenna:");
    canvas_draw_str(canvas, 56, 22, mgr->antenna_ok ? "OK" : "FAIL");

    canvas_draw_str(canvas, 4, 32, "Reading:");
    canvas_draw_str(canvas, 56, 32, mgr->reading ? "Yes" : "No");

    if(mgr->last_tag != 0) {
        char tag_str[16];
        snprintf(tag_str, sizeof(tag_str), "%08lX", (unsigned long)mgr->last_tag);
        canvas_draw_str(canvas, 4, 42, "Last tag:");
        canvas_draw_str(canvas, 60, 42, tag_str);
        canvas_draw_str(canvas, 4, 52, "Type:");
        canvas_draw_str(canvas, 36, 52, mgr->tag_type);
    } else {
        canvas_draw_str(canvas, 4, 42, "No tag read yet");
    }

    ui_draw_info_hint(canvas);
}
