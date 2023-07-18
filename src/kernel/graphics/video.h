#ifndef VIDEO_H

#include "../../std/memory.h"
#include "../x86.h"

#define VIDEO_H

void init_graphics();
void blit_pixels(uint8_t far *pixels, uint32_t sz);
void draw_character(uint8_t color, uint8_t c, uint32_t x, uint32_t y);
void put_pixel(uint8_t color, uint32_t x, uint32_t y);

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 200
#define TEXT_WIDTH 40
#define TEXT_HEIGHT 25

#endif
