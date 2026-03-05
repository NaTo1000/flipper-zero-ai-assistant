#include "../inc/http_client.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <furi_hal_serial.h>
#include <string.h>
#include <stdio.h>

#define HTTP_SERIAL_TIMEOUT_MS 10000
#define HTTP_BAUD              115200

HttpClient* http_client_alloc() {
    HttpClient* c = malloc(sizeof(HttpClient));
    if(c) {
        memset(c, 0, sizeof(HttpClient));
        c->method = HttpMethodGet;
    }
    return c;
}

void http_client_free(HttpClient* client) {
    if(client) free(client);
}

/* ── Internal serial request helper ─────────────────────────────────── */

/*
 * Send a FlipperHTTP HTTP command over USART and collect the response.
 * FlipperHTTP command format:
 *   [HTTP/GET]{"url":"...","headers":"..."}
 *   [HTTP/POST]{"url":"...","headers":"...","body":"..."}
 * Response: [HTTP/RESPONSE]{"code":200,"body":"..."}
 */
static bool http_serial_request(
    HttpClient* client,
    const char* cmd_tag,
    const char* json_payload) {
    client->in_progress = true;
    client->response_code = 0;
    memset(client->response, 0, sizeof(client->response));

    FuriHalSerialHandle* h = furi_hal_serial_control_acquire(FuriHalSerialIdUsart);
    if(!h) {
        client->in_progress = false;
        return false;
    }
    furi_hal_serial_init(h, HTTP_BAUD);

    /* Build command string: [CMD]{...}\r\n */
    char cmd[1600];
    int cmd_len =
        snprintf(cmd, sizeof(cmd), "%s%s\r\n", cmd_tag, json_payload ? json_payload : "{}");
    if(cmd_len <= 0 || cmd_len >= (int)sizeof(cmd)) {
        furi_hal_serial_deinit(h);
        furi_hal_serial_control_release(h);
        client->in_progress = false;
        return false;
    }

    furi_hal_serial_tx(h, (const uint8_t*)cmd, cmd_len);
    furi_hal_serial_tx_wait_complete(h);

    /* Read response lines until [HTTP/RESPONSE] is found */
    uint8_t rx_buf[256];
    size_t rx_len = 0;
    uint32_t t0 = furi_get_tick();
    bool found = false;

    while((furi_get_tick() - t0) < (uint32_t)furi_ms_to_ticks(HTTP_SERIAL_TIMEOUT_MS)) {
        if(furi_hal_serial_async_rx_available(h)) {
            uint8_t b = furi_hal_serial_async_rx(h);
            if(rx_len < sizeof(rx_buf) - 1) rx_buf[rx_len++] = b;
            if(b == '\n') {
                rx_buf[rx_len] = '\0';
                char* line = (char*)rx_buf;
                if(strstr(line, "[HTTP/RESPONSE]")) {
                    /* Parse response code */
                    char* code_ptr = strstr(line, "\"code\":");
                    if(code_ptr) {
                        client->response_code = atoi(code_ptr + 7);
                    }
                    /* Extract body */
                    char* body_ptr = strstr(line, "\"body\":\"");
                    if(body_ptr) {
                        body_ptr += 8;
                        /* Copy up to response buffer, un-escape \" */
                        size_t ri = 0;
                        while(*body_ptr && *body_ptr != '"' && ri < sizeof(client->response) - 1) {
                            if(*body_ptr == '\\' && *(body_ptr + 1) == '"') {
                                client->response[ri++] = '"';
                                body_ptr += 2;
                            } else {
                                client->response[ri++] = *body_ptr++;
                            }
                        }
                        client->response[ri] = '\0';
                    }
                    found = true;
                    break;
                }
                rx_len = 0;
            }
        }
        furi_delay_ms(5);
    }

    furi_hal_serial_deinit(h);
    furi_hal_serial_control_release(h);
    client->in_progress = false;
    return found;
}

/* ── Public API ──────────────────────────────────────────────────────── */

bool http_client_get(HttpClient* client, const char* url, const char* headers) {
    if(!client || !url) return false;
    snprintf(client->url, sizeof(client->url), "%s", url);

    char payload[600];
    snprintf(
        payload,
        sizeof(payload),
        "{\"url\":\"%s\",\"headers\":\"%s\"}",
        url,
        headers ? headers : "");
    return http_serial_request(client, "[HTTP/GET]", payload);
}

bool http_client_post(
    HttpClient* client,
    const char* url,
    const char* headers,
    const char* body) {
    if(!client || !url) return false;
    snprintf(client->url, sizeof(client->url), "%s", url);
    if(body) snprintf(client->body, sizeof(client->body), "%s", body);

    char payload[1200];
    snprintf(
        payload,
        sizeof(payload),
        "{\"url\":\"%s\",\"headers\":\"%s\",\"body\":\"%s\"}",
        url,
        headers ? headers : "",
        body ? body : "");
    return http_serial_request(client, "[HTTP/POST]", payload);
}

bool http_client_put(
    HttpClient* client,
    const char* url,
    const char* headers,
    const char* body) {
    if(!client || !url) return false;
    snprintf(client->url, sizeof(client->url), "%s", url);
    if(body) snprintf(client->body, sizeof(client->body), "%s", body);

    char payload[1200];
    snprintf(
        payload,
        sizeof(payload),
        "{\"url\":\"%s\",\"headers\":\"%s\",\"body\":\"%s\"}",
        url,
        headers ? headers : "",
        body ? body : "");
    return http_serial_request(client, "[HTTP/PUT]", payload);
}

bool http_client_delete(HttpClient* client, const char* url, const char* headers) {
    if(!client || !url) return false;
    snprintf(client->url, sizeof(client->url), "%s", url);

    char payload[600];
    snprintf(
        payload,
        sizeof(payload),
        "{\"url\":\"%s\",\"headers\":\"%s\"}",
        url,
        headers ? headers : "");
    return http_serial_request(client, "[HTTP/DELETE]", payload);
}

void http_client_draw_response(Canvas* canvas, HttpClient* client) {
    ui_draw_header(canvas, "HTTP Client");
    if(!client) return;

    canvas_set_font(canvas, FontSecondary);

    if(client->in_progress) {
        canvas_draw_str(canvas, 4, 32, "Request in progress...");
        return;
    }

    char code_str[16];
    snprintf(code_str, sizeof(code_str), "Code: %d", client->response_code);
    canvas_draw_str(canvas, 4, 22, code_str);

    /* Show first 60 characters of response body */
    char preview[61];
    snprintf(preview, sizeof(preview), "%.60s", client->response);
    ui_draw_wrapped_text(canvas, 4, 32, 120, preview);

    ui_draw_info_hint(canvas);
}
