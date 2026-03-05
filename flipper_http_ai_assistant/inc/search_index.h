#pragma once
#include <furi.h>

typedef struct {
    uint8_t chapter;
    uint16_t position;
    char keyword[32];
} SearchEntry;

typedef struct {
    SearchEntry* entries;
    uint16_t count;
} SearchIndex;

SearchIndex* search_index_alloc();
void search_index_free(SearchIndex* index);
void search_index_build(SearchIndex* index);
uint8_t search_index_find(
    SearchIndex* index,
    const char* query,
    uint8_t* results,
    uint8_t max_results);
