#include "../inc/search_index.h"
#include "../inc/manual_chapters.h"
#include <furi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Keywords extracted from chapter content for fast lookup */

/* Shared case-conversion helper */
static char char_to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
}

static const char* keyword_table[] = {
    "wifi",       "connect",    "http",       "bluetooth", "ble",
    "nrf",        "subghz",     "rfid",       "lf",        "json",
    "websocket",  "diagnostic", "troubleshoot","serial",   "uart",
    "usart",      "pin",        "gpio",       "firmware",  "esp32",
    "install",    "setup",      "scan",       "protocol",  "frequency",
    "antenna",    "channel",    "password",   "ssid",      "error",
    NULL
};

SearchIndex* search_index_alloc() {
    SearchIndex* idx = malloc(sizeof(SearchIndex));
    if(idx) {
        memset(idx, 0, sizeof(SearchIndex));
        /* Pre-allocate for MANUAL_CHAPTER_COUNT × keyword entries */
        idx->entries = malloc(sizeof(SearchEntry) * MANUAL_CHAPTER_COUNT * 30);
        idx->count = 0;
    }
    return idx;
}

void search_index_free(SearchIndex* index) {
    if(!index) return;
    if(index->entries) free(index->entries);
    free(index);
}

static bool keyword_in_text(const char* text, const char* kw) {
    if(!text || !kw) return false;
    size_t klen = strlen(kw);
    const char* p = text;
    while(*p) {
        bool match = true;
        for(size_t i = 0; i < klen; i++) {
            if(!p[i] || char_to_lower(p[i]) != char_to_lower(kw[i])) {
                match = false;
                break;
            }
        }
        if(match) return true;
        p++;
    }
    return false;
}

void search_index_build(SearchIndex* index) {
    if(!index || !index->entries) return;
    index->count = 0;
    uint16_t max_entries = MANUAL_CHAPTER_COUNT * 30;

    for(uint8_t ch = 0; ch < MANUAL_CHAPTER_COUNT; ch++) {
        const char* title = manual_get_chapter_title(ch);
        const char* content = manual_get_chapter_content(ch);

        for(int ki = 0; keyword_table[ki] != NULL; ki++) {
            if(index->count >= max_entries) break;
            if(keyword_in_text(title, keyword_table[ki]) ||
               keyword_in_text(content, keyword_table[ki])) {
                SearchEntry* e = &index->entries[index->count++];
                e->chapter = ch;
                e->position = 0;
                snprintf(e->keyword, sizeof(e->keyword), "%s", keyword_table[ki]);
            }
        }
    }
}

uint8_t search_index_find(
    SearchIndex* index,
    const char* query,
    uint8_t* results,
    uint8_t max_results) {
    if(!index || !query || !results || max_results == 0) return 0;

    uint8_t found = 0;
    /* Track which chapters have already been added to avoid duplicates */
    bool chapter_added[MANUAL_CHAPTER_COUNT];
    memset(chapter_added, 0, sizeof(chapter_added));

    for(uint16_t i = 0; i < index->count && found < max_results; i++) {
        SearchEntry* e = &index->entries[i];
        if(chapter_added[e->chapter]) continue;

        size_t qlen = strlen(query);
        size_t klen = strlen(e->keyword);
        /* Check if query matches keyword prefix (case-insensitive) */
        if(qlen <= klen) {
            bool match = true;
            for(size_t j = 0; j < qlen; j++) {
                if(char_to_lower(query[j]) != char_to_lower(e->keyword[j])) {
                    match = false;
                    break;
                }
            }
            if(match) {
                results[found++] = e->chapter;
                chapter_added[e->chapter] = true;
            }
        }
    }
    return found;
}
