#pragma once
#include <furi.h>
#include <gui/gui.h>

typedef enum {
    ModuleStatusUnknown,
    ModuleStatusOK,
    ModuleStatusNotFound,
    ModuleStatusError
} ModuleStatus;

typedef struct {
    ModuleStatus wifi;
    ModuleStatus ble;
    ModuleStatus nrf;
    ModuleStatus lf;
    ModuleStatus subghz;
    char wifi_version[32];
    char ble_version[32];
    int8_t wifi_rssi;
    bool diagnostics_complete;
} DiagnosticsReport;

DiagnosticsReport* diagnostics_alloc();
void diagnostics_free(DiagnosticsReport* report);
void diagnostics_run(DiagnosticsReport* report);
void diagnostics_draw(Canvas* canvas, DiagnosticsReport* report);
const char* module_status_icon(ModuleStatus status);
