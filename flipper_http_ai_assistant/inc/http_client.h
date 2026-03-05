#pragma once
#include <furi.h>

typedef enum {
    HttpMethodGet,
    HttpMethodPost,
    HttpMethodPut,
    HttpMethodDelete
} HttpMethod;

typedef struct {
    HttpMethod method;
    char url[256];
    char headers[512];
    char body[1024];
    char response[2048];
    int response_code;
    bool in_progress;
} HttpClient;

HttpClient* http_client_alloc();
void http_client_free(HttpClient* client);
bool http_client_get(HttpClient* client, const char* url, const char* headers);
bool http_client_post(HttpClient* client, const char* url, const char* headers, const char* body);
bool http_client_put(HttpClient* client, const char* url, const char* headers, const char* body);
bool http_client_delete(HttpClient* client, const char* url, const char* headers);
void http_client_draw_response(Canvas* canvas, HttpClient* client);
