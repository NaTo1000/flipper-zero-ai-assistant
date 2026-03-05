#include "../inc/json_parser.h"
#include "../inc/ui_helpers.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>

/*
 * Minimal JSON parser sufficient for FlipperHTTP response processing.
 * Handles flat key-value objects: {"key":"value","key2":123}
 * No recursive object/array nesting is required for FlipperHTTP.
 */

JsonParser* json_parser_alloc() {
    JsonParser* p = malloc(sizeof(JsonParser));
    if(p) memset(p, 0, sizeof(JsonParser));
    return p;
}

void json_parser_free(JsonParser* parser) {
    if(parser) free(parser);
}

bool json_parser_parse(JsonParser* parser, const char* json) {
    if(!parser || !json) return false;
    snprintf(parser->input, sizeof(parser->input), "%s", json);
    parser->parse_ok = (json[0] == '{' || json[0] == '[');
    return parser->parse_ok;
}

/*
 * Extract a string or numeric value for the given key from a JSON object.
 * Supports: "key":"value"  and  "key":number
 */
bool json_parser_get_value(
    JsonParser* parser,
    const char* key,
    char* value,
    size_t value_size) {
    if(!parser || !key || !value || value_size == 0) return false;
    if(!parser->parse_ok) return false;

    /* Build search pattern: "key": */
    char search[80];
    snprintf(search, sizeof(search), "\"%s\":", key);

    const char* pos = strstr(parser->input, search);
    if(!pos) return false;

    pos += strlen(search);
    /* Skip whitespace */
    while(*pos == ' ' || *pos == '\t') pos++;

    bool is_string = (*pos == '"');
    if(is_string) {
        pos++; /* skip opening quote */
        size_t i = 0;
        while(*pos && *pos != '"' && i < value_size - 1) {
            if(*pos == '\\' && *(pos + 1)) {
                pos++; /* skip escape */
                switch(*pos) {
                case 'n':
                    value[i++] = '\n';
                    break;
                case 't':
                    value[i++] = '\t';
                    break;
                case '"':
                    value[i++] = '"';
                    break;
                case '\\':
                    value[i++] = '\\';
                    break;
                default:
                    value[i++] = *pos;
                    break;
                }
            } else {
                value[i++] = *pos;
            }
            pos++;
        }
        value[i] = '\0';
    } else {
        /* Numeric / boolean / null */
        size_t i = 0;
        while(*pos && *pos != ',' && *pos != '}' && *pos != ']' && i < value_size - 1) {
            value[i++] = *pos++;
        }
        value[i] = '\0';
    }

    /* Cache for draw */
    snprintf(parser->last_key, sizeof(parser->last_key), "%s", key);
    snprintf(parser->last_value, sizeof(parser->last_value), "%s", value);
    return true;
}

bool json_parser_parse_array(JsonParser* parser, const char* json, const char* key) {
    if(!parser || !json || !key) return false;
    if(!json_parser_parse(parser, json)) return false;

    /* Extract the array string for the given key */
    char search[80];
    snprintf(search, sizeof(search), "\"%s\":[", key);
    const char* start = strstr(parser->input, search);
    if(!start) return false;

    start += strlen(search) - 1; /* position at '[' */
    const char* end = strchr(start, ']');
    if(!end) return false;

    size_t arr_len = end - start + 1;
    if(arr_len < sizeof(parser->output)) {
        memcpy(parser->output, start, arr_len);
        parser->output[arr_len] = '\0';
    }
    return true;
}

void json_parser_draw_result(Canvas* canvas, JsonParser* parser) {
    ui_draw_header(canvas, "JSON Parser");
    if(!parser) return;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 4, 22, "Status:");
    canvas_draw_str(canvas, 48, 22, parser->parse_ok ? "OK" : "FAIL");

    if(parser->last_key[0]) {
        char key_line[36];
        snprintf(key_line, sizeof(key_line), "Key: %.28s", parser->last_key);
        canvas_draw_str(canvas, 4, 32, key_line);
    }

    if(parser->last_value[0]) {
        char val_line[36];
        snprintf(val_line, sizeof(val_line), "Val: %.28s", parser->last_value);
        canvas_draw_str(canvas, 4, 42, val_line);
    }

    /* Show first fragment of parsed input */
    char input_preview[28];
    snprintf(input_preview, sizeof(input_preview), "%.27s", parser->input);
    canvas_draw_str(canvas, 4, 52, input_preview);

    ui_draw_info_hint(canvas);
}
