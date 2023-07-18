#pragma once
#include "memdefs.h"
#include "stddef.h"
#include "stdint.h"
typedef struct
{
	char far *begin;
	uint32_t size;
} linear_allocator;

typedef struct
{
	char far *ptr;
	size_t size;
	bool used;
} block_t;

typedef struct
{
	char far *memory;
	uint32_t number_of_blocks;
	block_t far *blocks;
} m_allocator_t;

m_allocator_t global_m_allocator;

void init_allocator();
void far *malloc(uint32_t bytes);
void free(void far *ptr);

void far *memcpy(void far *dst, const void far *src, uint16_t num);
void far *memset(void far *ptr, int value, uint16_t num);
int memcmp(const void far *ptr1, const void far *ptr2, uint16_t num);
void far *linear_allocate(linear_allocator *allocator, uint32_t bytes);
void sanitize_blocks();
void debug_print_blocks();
bool strcmp(char far *a, char far *b);
