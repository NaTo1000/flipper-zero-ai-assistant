#include "../inc/ai_agent.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <furi_hal_serial.h>
#include <string.h>
#include <stdio.h>

#define AI_SERIAL_TIMEOUT_MS 15000

AiAgent* ai_agent_alloc(WiFiManager* wifi) {
    AiAgent* a = malloc(sizeof(AiAgent));
    if(a) {
        memset(a, 0, sizeof(AiAgent));
        a->state = AgentStateIdle;
        a->wifi = wifi;
        snprintf(a->status_message, sizeof(a->status_message), "Ready");
    }
    return a;
}

void ai_agent_free(AiAgent* agent) {
    if(agent) free(agent);
}

/*
 * Automated WiFi setup sequence:
 *  1. Send [WIFI/SCAN] and collect networks
 *  2. Attempt connection to saved credentials (if any)
 *  3. Verify connectivity with [PING]
 *  4. Report success or guide user to configure manually
 */
void ai_agent_run_auto_install(AiAgent* agent) {
    if(!agent) return;

    agent->state = AgentStateScanning;
    agent->progress = 10;
    snprintf(agent->status_message, sizeof(agent->status_message), "Scanning for networks...");

    if(agent->wifi) {
        bool scan_ok = wifi_manager_scan(agent->wifi);
        agent->progress = 30;

        if(!scan_ok) {
            agent->state = AgentStateError;
            snprintf(
                agent->status_message,
                sizeof(agent->status_message),
                "No FlipperHTTP board found");
            return;
        }

        agent->state = AgentStateConnecting;
        agent->progress = 50;
        snprintf(agent->status_message, sizeof(agent->status_message), "Connecting...");

        /* Try saved credentials */
        bool conn_ok = false;
        if(agent->wifi->ssid[0]) {
            conn_ok =
                wifi_manager_connect(agent->wifi, agent->wifi->ssid, agent->wifi->password);
        }
        agent->progress = 70;

        if(!conn_ok) {
            agent->state = AgentStateError;
            snprintf(
                agent->status_message,
                sizeof(agent->status_message),
                "Connect failed - configure WiFi");
            return;
        }

        agent->state = AgentStateVerifying;
        agent->progress = 85;
        snprintf(agent->status_message, sizeof(agent->status_message), "Verifying...");

        bool ping_ok = wifi_manager_ping(agent->wifi, "8.8.8.8");
        agent->progress = 100;

        if(ping_ok) {
            agent->state = AgentStateComplete;
            agent->auto_install_done = true;
            snprintf(
                agent->status_message, sizeof(agent->status_message), "Setup complete!");
        } else {
            agent->state = AgentStateError;
            snprintf(
                agent->status_message,
                sizeof(agent->status_message),
                "No internet - check router");
        }
    } else {
        agent->state = AgentStateError;
        snprintf(agent->status_message, sizeof(agent->status_message), "WiFi manager not ready");
    }
}

void ai_agent_draw_status(Canvas* canvas, AiAgent* agent) {
    ui_draw_header(canvas, "AI Auto-Setup");
    if(!agent) return;

    canvas_set_font(canvas, FontSecondary);

    const char* state_str = "Idle";
    switch(agent->state) {
    case AgentStateScanning:
        state_str = "Scanning";
        break;
    case AgentStateConnecting:
        state_str = "Connecting";
        break;
    case AgentStateVerifying:
        state_str = "Verifying";
        break;
    case AgentStateComplete:
        state_str = "Complete";
        break;
    case AgentStateError:
        state_str = "Error";
        break;
    default:
        break;
    }

    canvas_draw_str(canvas, 4, 22, "Stage:");
    canvas_draw_str(canvas, 44, 22, state_str);

    ui_draw_wrapped_text(canvas, 4, 32, 120, agent->status_message);

    ui_draw_progress_bar(canvas, 4, 52, 120, 8, agent->progress);

    ui_draw_info_hint(canvas);
}
