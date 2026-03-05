#pragma once

#define MANUAL_CHAPTER_COUNT 13

extern const char* manual_chapter_titles[MANUAL_CHAPTER_COUNT];
extern const char* manual_chapter_content[MANUAL_CHAPTER_COUNT];

const char* manual_get_chapter_title(uint8_t index);
const char* manual_get_chapter_content(uint8_t index);
