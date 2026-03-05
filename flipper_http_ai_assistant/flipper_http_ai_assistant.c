/**
 * FlipperHTTP AI Assistant
 *
 * Main entry point and application lifecycle manager.
 * Architecture:
 *   - Single ViewPort for all rendering (manual scene dispatch)
 *   - Mutex-protected state shared between render and input callbacks
 *   - All manager modules are allocated/freed here
 */

#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <string.h>
#include <stdio.h>

#include "inc/diagnostics.h"
#include "inc/wifi_manager.h"
#include "inc/ble_manager.h"
#include "inc/nrf_manager.h"
#include "inc/lf_manager.h"
#include "inc/subghz_manager.h"
#include "inc/http_client.h"
#include "inc/websocket_client.h"
#include "inc/json_parser.h"
#include "inc/ai_agent.h"
#include "inc/manual.h"
#include "inc/manual_chapters.h"
#include "inc/search_index.h"
#include "inc/info_overlay.h"
#include "inc/ui_helpers.h"

/* ── Constants ──────────────────────────────────────────────────────────── */
#define EVENT_QUEUE_SIZE 16

/* ── Scene IDs ──────────────────────────────────────────────────────────── */

typedef enum {
    SceneSplash = 0,
    SceneDiagnostics,
    SceneMainMenu,
    SceneWifi,
    SceneBle,
    SceneNrf,
    SceneLf,
    SceneSubGhz,
    SceneHttp,
    SceneWebSocket,
    SceneManualToc,
    SceneManualChapter,
    SceneManualSearch,
    SceneAiSetup,
    SceneCount
} Scene;

/* ── Main menu items ────────────────────────────────────────────────────── */

static const char* const MENU_ITEMS[] = {
    "WiFi Manager",
    "AI Auto-Setup",
    "HTTP Client",
    "WebSocket",
    "BLE Manager",
    "nRF24 Module",
    "LF RFID",
    "Sub-GHz",
    "Tutorial Manual",
    "Diagnostics",
};
#define MENU_ITEM_COUNT 10

/* Map menu index → scene */
static const Scene MENU_SCENES[MENU_ITEM_COUNT] = {
    SceneWifi,
    SceneAiSetup,
    SceneHttp,
    SceneWebSocket,
    SceneBle,
    SceneNrf,
    SceneLf,
    SceneSubGhz,
    SceneManualToc,
    SceneDiagnostics,
};

/* ── App state ──────────────────────────────────────────────────────────── */

typedef struct {
    /* GUI */
    Gui* gui;
    ViewPort* viewport;
    FuriMessageQueue* event_queue;
    FuriMutex* mutex;

    /* Scene */
    Scene current_scene;
    Scene previous_scene;

    /* Splash */
    uint32_t splash_start_tick;
    bool splash_done;

    /* Main menu */
    uint8_t menu_index;
    uint8_t menu_scroll;

    /* Modules */
    DiagnosticsReport* diagnostics;
    WiFiManager* wifi;
    BleManager* ble;
    NrfManager* nrf;
    LfManager* lf;
    SubGhzManager* subghz;
    HttpClient* http;
    WebSocketClient* websocket;
    JsonParser* json;
    AiAgent* ai_agent;
    ManualState* manual;
    SearchIndex* search_index;
    InfoOverlay* info_overlay;

    /* Per-scene state */
    bool diagnostics_started;
    bool ai_started;

    /* Info overlay context for current scene */
    InfoContext info_context;

    bool running;
} AppState;

/* ── Forward declarations ───────────────────────────────────────────────── */

static void app_draw_callback(Canvas* canvas, void* ctx);
static void app_input_callback(InputEvent* event, void* ctx);
static void scene_transition(AppState* app, Scene next);

/* ── Draw helpers ───────────────────────────────────────────────────────── */

static void draw_splash(Canvas* canvas, AppState* app) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 18, 20, "FlipperHTTP AI");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 22, 32, "WiFi Assistant v1.0");
    canvas_draw_str(canvas, 30, 44, "by FlipperHTTP");
    /* Draw simple dolphin */
    ui_draw_dolphin_welcome(canvas, 0, 14);
    (void)app;
}

static void draw_main_menu(Canvas* canvas, AppState* app) {
    ui_draw_header(canvas, "FlipperHTTP AI");
    canvas_set_font(canvas, FontSecondary);

    uint8_t visible = 5;
    for(uint8_t i = 0; i < visible; i++) {
        uint8_t idx = app->menu_scroll + i;
        if(idx >= MENU_ITEM_COUNT) break;
        int y = 14 + i * 10;
        if(idx == app->menu_index) {
            canvas_draw_box(canvas, 0, y - 8, 122, 10);
            canvas_set_color(canvas, ColorWhite);
        }
        canvas_draw_str(canvas, 4, y, MENU_ITEMS[idx]);
        canvas_set_color(canvas, ColorBlack);
    }
    ui_draw_scrollbar(canvas, MENU_ITEM_COUNT, visible, app->menu_scroll);
    ui_draw_info_hint(canvas);
}

/* ── Main draw callback ─────────────────────────────────────────────────── */

static void app_draw_callback(Canvas* canvas, void* ctx) {
    AppState* app = (AppState*)ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    canvas_clear(canvas);

    /* If info overlay is active, render it on top of whatever scene */
    if(app->info_overlay && app->info_overlay->visible) {
        /* Draw current scene as background first */
        switch(app->current_scene) {
        case SceneSplash:
            draw_splash(canvas, app);
            break;
        case SceneMainMenu:
            draw_main_menu(canvas, app);
            break;
        default:
            break;
        }
        info_overlay_draw(canvas, app->info_overlay);
        furi_mutex_release(app->mutex);
        return;
    }

    switch(app->current_scene) {
    case SceneSplash:
        draw_splash(canvas, app);
        break;

    case SceneDiagnostics:
        if(app->diagnostics) {
            diagnostics_draw(canvas, app->diagnostics);
        }
        break;

    case SceneMainMenu:
        draw_main_menu(canvas, app);
        break;

    case SceneWifi:
        if(app->wifi) wifi_manager_draw_status(canvas, app->wifi);
        break;

    case SceneBle:
        if(app->ble) ble_manager_draw_status(canvas, app->ble);
        break;

    case SceneNrf:
        if(app->nrf) nrf_manager_draw_status(canvas, app->nrf);
        break;

    case SceneLf:
        if(app->lf) lf_manager_draw_status(canvas, app->lf);
        break;

    case SceneSubGhz:
        if(app->subghz) subghz_manager_draw_status(canvas, app->subghz);
        break;

    case SceneHttp:
        if(app->http) http_client_draw_response(canvas, app->http);
        break;

    case SceneWebSocket:
        if(app->websocket) websocket_client_draw_status(canvas, app->websocket);
        break;

    case SceneManualToc:
        if(app->manual) manual_draw_toc(canvas, app->manual);
        break;

    case SceneManualChapter:
        if(app->manual) manual_draw_chapter(canvas, app->manual);
        break;

    case SceneManualSearch:
        if(app->manual) manual_draw_search(canvas, app->manual);
        break;

    case SceneAiSetup:
        if(app->ai_agent) ai_agent_draw_status(canvas, app->ai_agent);
        break;

    default:
        canvas_draw_str(canvas, 4, 32, "Unknown scene");
        break;
    }

    furi_mutex_release(app->mutex);
}

/* ── Scene transition ───────────────────────────────────────────────────── */

static void scene_transition(AppState* app, Scene next) {
    app->previous_scene = app->current_scene;
    app->current_scene = next;

    /* Set info context for new scene */
    static const InfoContext scene_info_map[SceneCount] = {
        [SceneSplash] = InfoContextSplash,
        [SceneDiagnostics] = InfoContextDiagnostics,
        [SceneMainMenu] = InfoContextMainMenu,
        [SceneWifi] = InfoContextWifi,
        [SceneBle] = InfoContextBle,
        [SceneNrf] = InfoContextNrf,
        [SceneLf] = InfoContextLf,
        [SceneSubGhz] = InfoContextSubGhz,
        [SceneHttp] = InfoContextHttp,
        [SceneWebSocket] = InfoContextWebSocket,
        [SceneManualToc] = InfoContextManual,
        [SceneManualChapter] = InfoContextManual,
        [SceneManualSearch] = InfoContextSearch,
        [SceneAiSetup] = InfoContextMainMenu,
    };
    app->info_context = scene_info_map[next];
}

/* ── Input handler ──────────────────────────────────────────────────────── */

static void app_input_callback(InputEvent* event, void* ctx) {
    AppState* app = (AppState*)ctx;
    furi_message_queue_put(app->event_queue, event, 0);
}

static void handle_input(AppState* app, InputEvent* event) {
    /* ── Info overlay intercepts all input when visible ──────────────── */
    if(app->info_overlay && app->info_overlay->visible) {
        if(event->type == InputTypeShort || event->type == InputTypeRepeat) {
            switch(event->key) {
            case InputKeyUp:
                info_overlay_scroll_up(app->info_overlay);
                break;
            case InputKeyDown:
                info_overlay_scroll_down(app->info_overlay);
                break;
            case InputKeyOk:
            case InputKeyBack:
                info_overlay_hide(app->info_overlay);
                break;
            default:
                break;
            }
        }
        return;
    }

    /* ── Long-press OK → show info overlay ──────────────────────────── */
    if(event->type == InputTypeLong && event->key == InputKeyOk) {
        info_overlay_show(app->info_overlay, app->info_context);
        return;
    }

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return;

    switch(app->current_scene) {

    /* ── Splash ─────────────────────────────────────────────────────── */
    case SceneSplash:
        /* Any key skips splash */
        scene_transition(app, SceneDiagnostics);
        app->diagnostics_started = false;
        break;

    /* ── Diagnostics ────────────────────────────────────────────────── */
    case SceneDiagnostics:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk) {
            /* Re-run diagnostics */
            if(app->diagnostics) {
                app->diagnostics_started = false;
            }
        }
        break;

    /* ── Main Menu ──────────────────────────────────────────────────── */
    case SceneMainMenu:
        switch(event->key) {
        case InputKeyUp:
            if(app->menu_index > 0) {
                app->menu_index--;
                if(app->menu_index < app->menu_scroll) app->menu_scroll = app->menu_index;
            }
            break;
        case InputKeyDown:
            if(app->menu_index < MENU_ITEM_COUNT - 1) {
                app->menu_index++;
                if(app->menu_index >= app->menu_scroll + 5)
                    app->menu_scroll = app->menu_index - 4;
            }
            break;
        case InputKeyOk:
            scene_transition(app, MENU_SCENES[app->menu_index]);
            /* Trigger AI setup on entry */
            if(MENU_SCENES[app->menu_index] == SceneAiSetup) {
                app->ai_started = false;
            }
            /* Refresh BLE status on entry */
            if(MENU_SCENES[app->menu_index] == SceneBle && app->ble) {
                ble_manager_get_status(app->ble);
            }
            /* Check nRF on entry */
            if(MENU_SCENES[app->menu_index] == SceneNrf && app->nrf) {
                nrf_manager_detect(app->nrf);
            }
            /* Check LF antenna on entry */
            if(MENU_SCENES[app->menu_index] == SceneLf && app->lf) {
                lf_manager_check_antenna(app->lf);
            }
            /* Check Sub-GHz on entry */
            if(MENU_SCENES[app->menu_index] == SceneSubGhz && app->subghz) {
                subghz_manager_check_radio(app->subghz);
            }
            break;
        case InputKeyBack:
            app->running = false;
            break;
        default:
            break;
        }
        break;

    /* ── WiFi ───────────────────────────────────────────────────────── */
    case SceneWifi:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk) {
            if(app->wifi && app->wifi->state == WiFiStateDisconnected) {
                wifi_manager_scan(app->wifi);
            } else if(app->wifi && app->wifi->state == WiFiStateConnected) {
                wifi_manager_disconnect(app->wifi);
            }
        }
        break;

    /* ── BLE ────────────────────────────────────────────────────────── */
    case SceneBle:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->ble) {
            if(app->ble->enabled) {
                ble_manager_disable(app->ble);
            } else {
                ble_manager_enable(app->ble);
            }
        }
        break;

    /* ── nRF ────────────────────────────────────────────────────────── */
    case SceneNrf:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->nrf) {
            nrf_manager_detect(app->nrf);
            if(app->nrf->detected) nrf_manager_configure(app->nrf);
        }
        break;

    /* ── LF ─────────────────────────────────────────────────────────── */
    case SceneLf:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->lf) {
            app->lf->reading = true;
            lf_manager_read_tag(app->lf);
        }
        break;

    /* ── Sub-GHz ────────────────────────────────────────────────────── */
    case SceneSubGhz:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->subghz) {
            subghz_manager_quick_test(app->subghz);
        }
        break;

    /* ── HTTP Client ────────────────────────────────────────────────── */
    case SceneHttp:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->http) {
            /* Demo: GET a test endpoint */
            http_client_get(app->http, "http://httpbin.org/get", "");
        }
        break;

    /* ── WebSocket ──────────────────────────────────────────────────── */
    case SceneWebSocket:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->websocket) {
            if(app->websocket->state == WebSocketStateClosed ||
               app->websocket->state == WebSocketStateError) {
                websocket_client_start(app->websocket, "wss://echo.websocket.org");
            } else {
                websocket_client_stop(app->websocket);
            }
        }
        break;

    /* ── Manual TOC ─────────────────────────────────────────────────── */
    case SceneManualToc:
        switch(event->key) {
        case InputKeyUp:
            manual_scroll_up(app->manual);
            if(app->manual->chapter_index > 0) {
                app->manual->chapter_index--;
                if(app->manual->chapter_index < app->manual->scroll_pos)
                    app->manual->scroll_pos = app->manual->chapter_index;
            }
            break;
        case InputKeyDown:
            manual_scroll_down(app->manual);
            if(app->manual->chapter_index < MANUAL_CHAPTER_COUNT - 1) {
                app->manual->chapter_index++;
                if(app->manual->chapter_index >= app->manual->scroll_pos + 5)
                    app->manual->scroll_pos = app->manual->chapter_index - 4;
            }
            break;
        case InputKeyOk:
            app->manual->scroll_pos = 0;
            scene_transition(app, SceneManualChapter);
            break;
        case InputKeyBack:
            scene_transition(app, SceneMainMenu);
            break;
        default:
            break;
        }
        break;

    /* ── Manual Chapter ─────────────────────────────────────────────── */
    case SceneManualChapter:
        switch(event->key) {
        case InputKeyUp:
            manual_scroll_up(app->manual);
            break;
        case InputKeyDown:
            manual_scroll_down(app->manual);
            break;
        case InputKeyBack:
            app->manual->scroll_pos = 0;
            scene_transition(app, SceneManualToc);
            break;
        default:
            break;
        }
        break;

    /* ── Manual Search ──────────────────────────────────────────────── */
    case SceneManualSearch:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneManualToc);
        }
        break;

    /* ── AI Setup ───────────────────────────────────────────────────── */
    case SceneAiSetup:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        } else if(event->key == InputKeyOk && app->ai_agent) {
            app->ai_started = false;
        }
        break;

    default:
        if(event->key == InputKeyBack) {
            scene_transition(app, SceneMainMenu);
        }
        break;
    }
}

/* ── App alloc / free ───────────────────────────────────────────────────── */

static AppState* app_alloc() {
    AppState* app = malloc(sizeof(AppState));
    if(!app) return NULL;
    memset(app, 0, sizeof(AppState));

    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->event_queue = furi_message_queue_alloc(EVENT_QUEUE_SIZE, sizeof(InputEvent));

    app->current_scene = SceneSplash;
    app->previous_scene = SceneSplash;
    app->info_context = InfoContextSplash;
    app->running = true;
    app->splash_start_tick = furi_get_tick();

    /* Allocate modules */
    app->diagnostics = diagnostics_alloc();
    app->wifi = wifi_manager_alloc();
    app->ble = ble_manager_alloc();
    app->nrf = nrf_manager_alloc();
    app->lf = lf_manager_alloc();
    app->subghz = subghz_manager_alloc();
    app->http = http_client_alloc();
    app->websocket = websocket_client_alloc();
    app->json = json_parser_alloc();
    app->ai_agent = ai_agent_alloc(app->wifi);
    app->manual = manual_alloc();
    app->search_index = search_index_alloc();
    app->info_overlay = info_overlay_alloc();

    /* Pre-build search index */
    if(app->search_index) search_index_build(app->search_index);

    /* GUI */
    app->viewport = view_port_alloc();
    view_port_draw_callback_set(app->viewport, app_draw_callback, app);
    view_port_input_callback_set(app->viewport, app_input_callback, app);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->viewport, GuiLayerFullscreen);

    return app;
}

static void app_free(AppState* app) {
    if(!app) return;

    /* Remove viewport first */
    if(app->gui && app->viewport) {
        gui_remove_view_port(app->gui, app->viewport);
    }
    if(app->viewport) view_port_free(app->viewport);
    if(app->gui) furi_record_close(RECORD_GUI);

    /* Free modules */
    if(app->diagnostics) diagnostics_free(app->diagnostics);
    if(app->wifi) wifi_manager_free(app->wifi);
    if(app->ble) ble_manager_free(app->ble);
    if(app->nrf) nrf_manager_free(app->nrf);
    if(app->lf) lf_manager_free(app->lf);
    if(app->subghz) subghz_manager_free(app->subghz);
    if(app->http) http_client_free(app->http);
    if(app->websocket) websocket_client_free(app->websocket);
    if(app->json) json_parser_free(app->json);
    if(app->ai_agent) ai_agent_free(app->ai_agent);
    if(app->manual) manual_free(app->manual);
    if(app->search_index) search_index_free(app->search_index);
    if(app->info_overlay) info_overlay_free(app->info_overlay);

    if(app->event_queue) furi_message_queue_free(app->event_queue);
    if(app->mutex) furi_mutex_free(app->mutex);

    free(app);
}

/* ── Main entry point ───────────────────────────────────────────────────── */

int32_t flipper_http_ai_assistant_app(void* p) {
    UNUSED(p);

    AppState* app = app_alloc();
    if(!app) return -1;

    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);

    /* Brief vibration to indicate app launch */
    notification_message(notifications, &sequence_blink_blue_10);

    InputEvent event;

    while(app->running) {
        /* ── Auto-transitions ──────────────────────────────────────── */
        furi_mutex_acquire(app->mutex, FuriWaitForever);

        /* Splash auto-advance after 2 seconds */
        if(app->current_scene == SceneSplash) {
            if(furi_get_tick() - app->splash_start_tick > furi_ms_to_ticks(2000)) {
                scene_transition(app, SceneDiagnostics);
                app->diagnostics_started = false;
            }
        }

        /* Run diagnostics once on entry */
        if(app->current_scene == SceneDiagnostics && !app->diagnostics_started) {
            app->diagnostics_started = true;
            furi_mutex_release(app->mutex);
            /* Run outside mutex to allow render updates */
            if(app->diagnostics) diagnostics_run(app->diagnostics);
            furi_mutex_acquire(app->mutex, FuriWaitForever);
        }

        /* Run AI auto-install once on entry */
        if(app->current_scene == SceneAiSetup && !app->ai_started) {
            app->ai_started = true;
            furi_mutex_release(app->mutex);
            if(app->ai_agent) ai_agent_run_auto_install(app->ai_agent);
            furi_mutex_acquire(app->mutex, FuriWaitForever);
        }

        furi_mutex_release(app->mutex);

        /* ── Process input events ─────────────────────────────────── */
        if(furi_message_queue_get(app->event_queue, &event, 50) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            handle_input(app, &event);
            furi_mutex_release(app->mutex);
        }

        view_port_update(app->viewport);
    }

    notification_message(notifications, &sequence_blink_blue_10);
    furi_record_close(RECORD_NOTIFICATION);

    app_free(app);
    return 0;
}
