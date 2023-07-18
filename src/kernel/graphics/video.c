#include "video.h"
void init_graphics() { x86_Set_Video_Mode(0x13); }
void blit_pixels(uint8_t far *pixels, uint32_t sz)
{
	char far *video_buffer = (char far *)0xA0000000L;
	memcpy(video_buffer, pixels, sz);
}

void put_pixel(uint8_t color, uint32_t x, uint32_t y)
{
	if (x > VIDEO_WIDTH || y > VIDEO_HEIGHT)
	{
		return;
	}
	x86_Put_Pixel(color, (uint16_t)x, (uint16_t)y);
}

void draw_character(uint8_t color, uint8_t c, uint32_t x, uint32_t y)
{
	x86_Set_Cursor_Pos((uint8_t)x, (uint8_t)y);
	x86_Video_WriteCharTeletype(color, c, 0);
}
