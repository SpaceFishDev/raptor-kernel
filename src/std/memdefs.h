#pragma once

// 0x00000000 - 0x000003FF - interrupt vector table
// 0x00000400 - 0x000004FF - BIOS data area

#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

#define MALLOC_BEGIN ((void far *)0x01050000)
#define MALLOC_END ((void far *)0x00080000)

#define MEMORY_FAT_ADDR ((void far *)0x00500000)  // segment:offset (SSSSOOOO)
#define MEMORY_FAT_SIZE 0x00010000
#define NULL (void *)0
#define REAL_ALLOCATOR_BEGIN (void far *)((2000 * 1024) + 0x7c00)
#define ALLOCATOR_MAX_SIZE (1024 * 32)
#define IDT_LOCATION 0x10000

#define FP_OFF(p) ((unsigned)((unsigned long)(p)&0xFFFF))
#define FP_SEG(p) ((unsigned)((unsigned long)(p) >> 16))
#define MK_FP(seg, off) \
	((void far *)(((unsigned long)(seg) << 16) | (unsigned)(off)))
