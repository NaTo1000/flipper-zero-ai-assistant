#pragma once
#include <furi.h>
#include "wifi_manager.h"

typedef enum {
    AgentStateIdle,
    AgentStateScanning,
    AgentStateConnecting,
    AgentStateVerifying,
    AgentStateComplete,
    AgentStateError
} AgentState;

typedef struct {
    AgentState state;
    char status_message[128];
    uint8_t progress;
    bool auto_install_done;
    WiFiManager* wifi;
} AiAgent;

AiAgent* ai_agent_alloc(WiFiManager* wifi);
void ai_agent_free(AiAgent* agent);
void ai_agent_run_auto_install(AiAgent* agent);
void ai_agent_draw_status(Canvas* canvas, AiAgent* agent);
