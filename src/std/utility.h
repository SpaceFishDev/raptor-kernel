#pragma once
#include "stddef.h"
#include "stdint.h"
uint32_t align(uint32_t number, uint32_t alignTo);
bool islower(char chr);
char toupper(char chr);
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
