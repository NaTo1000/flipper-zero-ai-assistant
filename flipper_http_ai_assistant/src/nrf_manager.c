#include "../inc/nrf_manager.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>

/*
 * nRF24L01+ detection is performed over the Flipper GPIO SPI bus.
 * A real implementation would use furi_hal_spi to read the NRF CONFIG
 * register (0x00).  Here we provide the structural scaffolding and a
 * safe stub that reports "Unknown" so the app compiles and runs without
 * the add-on board attached.
 */

NrfManager* nrf_manager_alloc() {
    NrfManager* m = malloc(sizeof(NrfManager));
    if(m) {
        memset(m, 0, sizeof(NrfManager));
        m->channel = 76;
        m->power = 0; /* 0 dBm */
    }
    return m;
}

void nrf_manager_free(NrfManager* mgr) {
    if(mgr) free(mgr);
}

bool nrf_manager_detect(NrfManager* mgr) {
    if(!mgr) return false;
    /*
     * To detect nRF24: bring CS low, clock out 0x00 (READ | CONFIG),
     * read status + CONFIG byte.  CONFIG default = 0x08.
     * Without furi_hal_spi_bus_handle for the external GPIO pins we
     * cannot safely perform SPI here, so we report "not detected".
     */
    mgr->detected = false;
    return false;
}

bool nrf_manager_configure(NrfManager* mgr) {
    if(!mgr || !mgr->detected) return false;
    mgr->configured = false;
    return false;
}

bool nrf_manager_test(NrfManager* mgr) {
    if(!mgr || !mgr->configured) return false;
    return false;
}

void nrf_manager_draw_status(Canvas* canvas, NrfManager* mgr) {
    ui_draw_header(canvas, "nRF24 Manager");
    if(!mgr) return;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 22, "Detected:");
    canvas_draw_str(canvas, 60, 22, mgr->detected ? "Yes" : "No");

    canvas_draw_str(canvas, 4, 32, "Channel:");
    char ch_str[8];
    snprintf(ch_str, sizeof(ch_str), "%u", mgr->channel);
    canvas_draw_str(canvas, 56, 32, ch_str);

    canvas_draw_str(canvas, 4, 42, "Configured:");
    canvas_draw_str(canvas, 70, 42, mgr->configured ? "Yes" : "No");

    canvas_draw_str(canvas, 4, 52, "Address:");
    canvas_draw_str(canvas, 56, 52, mgr->address[0] ? mgr->address : "N/A");

    ui_draw_info_hint(canvas);
}
