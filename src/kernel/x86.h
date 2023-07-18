#pragma once
#include "../std/stddef.h"
#include "../std/stdint.h"

void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor,
						 uint64_t *quotientOut, uint32_t *remainderOut);

void _cdecl x86_Video_WriteCharTeletype(char color, char c, uint8_t page);

bool _cdecl x86_Disk_Reset(uint8_t drive);

bool _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector,
						  uint16_t head, uint8_t count, void far *dataOut);

bool _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t *driveTypeOut,
									uint16_t *cylindersOut,
									uint16_t *sectorsOut, uint16_t *headsOut);
bool _cdecl x86_Disk_Write_Sector(uint8_t drive, uint16_t cylinder,
								  uint16_t sector, uint16_t head, uint8_t count,
								  void far *dataIn);
void _cdecl x86_Set_Video_Mode(uint8_t mode);
void _cdecl x86_Put_Pixel(uint8_t color, uint16_t x, uint16_t y);
void _cdecl x86_Set_Cursor_Pos(uint8_t x, uint8_t y);
void _cdecl x86_Reboot();

char _cdecl inb(uint8_t port);
void _cdecl outb(uint8_t port, uint8_t byte);
void _cdecl install_idt_element(char *segment, char *offset);
unsigned short _cdecl getDS();
