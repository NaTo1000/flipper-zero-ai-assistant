#pragma once
#include <furi.h>

typedef enum {
    WebSocketStateClosed,
    WebSocketStateConnecting,
    WebSocketStateOpen,
    WebSocketStateError
} WebSocketState;

typedef struct {
    WebSocketState state;
    char url[256];
    char last_message[512];
    uint32_t messages_received;
} WebSocketClient;

WebSocketClient* websocket_client_alloc();
void websocket_client_free(WebSocketClient* client);
bool websocket_client_start(WebSocketClient* client, const char* url);
bool websocket_client_stop(WebSocketClient* client);
void websocket_client_draw_status(Canvas* canvas, WebSocketClient* client);
