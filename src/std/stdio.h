#ifndef STDIO_H

#include "stddef.h"
#include "stdint.h"

#define STDIO_H

void _putc(char c, char color);
uint32_t _puts(const char *str);
void puts_f(const char far *str);
void _cdecl printf(const char *fmt, ...);
uint32_t strlen(char *str);

#endif
