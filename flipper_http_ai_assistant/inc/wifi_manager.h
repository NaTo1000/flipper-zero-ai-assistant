#pragma once
#include <furi.h>

typedef enum {
    WiFiStateDisconnected,
    WiFiStateConnecting,
    WiFiStateConnected,
    WiFiStateError
} WiFiState;

typedef struct {
    WiFiState state;
    char ssid[64];
    char password[64];
    char ip_address[16];
    int8_t rssi;
    bool scan_in_progress;
    char** scan_results;
    uint8_t scan_count;
} WiFiManager;

WiFiManager* wifi_manager_alloc();
void wifi_manager_free(WiFiManager* mgr);
bool wifi_manager_connect(WiFiManager* mgr, const char* ssid, const char* password);
bool wifi_manager_disconnect(WiFiManager* mgr);
bool wifi_manager_scan(WiFiManager* mgr);
bool wifi_manager_get_ip(WiFiManager* mgr);
bool wifi_manager_ping(WiFiManager* mgr, const char* host);
bool wifi_manager_save_credentials(WiFiManager* mgr);
void wifi_manager_draw_status(Canvas* canvas, WiFiManager* mgr);
