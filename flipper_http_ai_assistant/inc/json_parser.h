#pragma once
#include <furi.h>

typedef struct {
    char input[2048];
    char output[2048];
    char last_key[64];
    char last_value[256];
    bool parse_ok;
} JsonParser;

JsonParser* json_parser_alloc();
void json_parser_free(JsonParser* parser);
bool json_parser_parse(JsonParser* parser, const char* json);
bool json_parser_get_value(JsonParser* parser, const char* key, char* value, size_t value_size);
bool json_parser_parse_array(JsonParser* parser, const char* json, const char* key);
void json_parser_draw_result(Canvas* canvas, JsonParser* parser);
