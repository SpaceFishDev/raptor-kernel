#include "../std/memory.h"
#include "../std/stdio.h"

linear_allocator temp_alloc;

void far *memcpy(void far *dst, const void far *src, uint16_t num)
{
	uint8_t far *u8Dst = (uint8_t far *)dst;
	const uint8_t far *u8Src = (const uint8_t far *)src;

	for (uint16_t i = 0; i < num; i++)
		u8Dst[i] = u8Src[i];

	return dst;
}

void far *memset(void far *ptr, int value, uint16_t num)
{
	uint8_t far *u8Ptr = (uint8_t far *)ptr;

	for (uint16_t i = 0; i < num; i++)
		u8Ptr[i] = (uint8_t)value;

	return ptr;
}

int memcmp(const void far *ptr1, const void far *ptr2, uint16_t num)
{
	const uint8_t far *u8Ptr1 = (const uint8_t far *)ptr1;
	const uint8_t far *u8Ptr2 = (const uint8_t far *)ptr2;

	for (uint16_t i = 0; i < num; i++)
		if (u8Ptr1[i] != u8Ptr2[i])
			return 1;

	return 0;
}
void far *linear_allocate(linear_allocator *allocator, uint32_t bytes)
{
	if (allocator->begin == 0)
	{
		allocator->begin = MALLOC_BEGIN;
		allocator->size = bytes;
		return allocator->begin;
	}
	void far *r = allocator->begin + allocator->size;
	allocator->size += bytes;
	return r;
}
void init_allocator()
{
	global_m_allocator.memory = REAL_ALLOCATOR_BEGIN;
	global_m_allocator.number_of_blocks = 1;
	global_m_allocator.blocks = (block_t far *)linear_allocate(&temp_alloc, sizeof(block_t));
	block_t starting_block;
	starting_block.ptr = global_m_allocator.memory;
	starting_block.size = ALLOCATOR_MAX_SIZE;
	starting_block.used = false;
	global_m_allocator.blocks[0] = starting_block;
}

void create_new_block(block_t block)
{
	global_m_allocator.number_of_blocks++;
	void far *old_blocks = (void far *)global_m_allocator.blocks;
	block_t far *new_blocks = linear_allocate(&temp_alloc, sizeof(block_t) * global_m_allocator.number_of_blocks);
	memcpy((void far *)new_blocks, old_blocks, sizeof(block_t) * (global_m_allocator.number_of_blocks - 1));
	new_blocks[global_m_allocator.number_of_blocks - 1] = block;
	global_m_allocator.blocks = new_blocks;
}

void block_swap(block_t far *a, block_t far *b)
{
	block_t temp = *a;
	*a = *b;
	*b = temp;
}

void bubbleSort(block_t far *arr, int size)
{
	for (int i = 0; i < size - 1; i++)
	{
		for (int j = 0; j < size - i - 1; j++)
		{
			if (!arr[j].used && arr[j + 1].used)
			{
				block_swap(&arr[j], &arr[j + 1]);
			}
		}
	}
}
void reverse_blocks(block_t far *arr, int n)
{
	block_t temp;
	for (int i = 0; i < n / 2; i++)
	{
		temp = arr[i];
		arr[i] = arr[n - i - 1];
		arr[n - i - 1] = temp;
	}
}

void debug_print_blocks()
{
	for (int i = 0; i < global_m_allocator.number_of_blocks; ++i)
	{
		printf("BLOCK: %d\n", i);
		printf("PTR: %u\n", (unsigned int)global_m_allocator.blocks[i].ptr);
		printf("SIZE: %u\n", global_m_allocator.blocks[i].size);
		printf("USED: %d\n", global_m_allocator.blocks[i].used);
		printf("\n");
	}
}

void far *malloc(uint32_t bytes)
{
	for (int i = 0; i < global_m_allocator.number_of_blocks; ++i)
	{
		if (global_m_allocator.blocks[i].used == false && global_m_allocator.blocks[i].size == bytes)
		{
			global_m_allocator.blocks[i].used = true;
			return global_m_allocator.blocks[i].ptr;
		}
		else if (global_m_allocator.blocks[i].used == false && global_m_allocator.blocks[i].size > bytes)
		{
			for (int j = i + 1; j < global_m_allocator.number_of_blocks; j++)
			{
				if (global_m_allocator.blocks[j].used == false && global_m_allocator.blocks[j].size >= bytes)
				{
					global_m_allocator.blocks[j].used = true;
					return global_m_allocator.blocks[j].ptr;
				}
			}
			block_t new_block;
			new_block.ptr = global_m_allocator.blocks[i].ptr + bytes;
			new_block.size = global_m_allocator.blocks[i].size - bytes;
			new_block.used = false;
			void far *ret = global_m_allocator.blocks[i].ptr;
			global_m_allocator.blocks[i] = new_block;
			block_t n;
			n.ptr = ret;
			n.size = bytes;
			n.used = true;
			create_new_block(n);
			return ret;
		}
	}
	return 0;
}

void free(void far *ptr)
{
	for (int i = 0; i < global_m_allocator.number_of_blocks; ++i)
	{
		if (global_m_allocator.blocks[i].ptr == ptr)
		{
			if (global_m_allocator.blocks[i].ptr + global_m_allocator.blocks[i].size == global_m_allocator.blocks[i + 1].ptr)
			{
				global_m_allocator.blocks[i].size = global_m_allocator.blocks[i + 1].size + global_m_allocator.blocks[i].size;
				for (int x = i; x < global_m_allocator.number_of_blocks; ++x)
				{
					global_m_allocator.blocks[x] = global_m_allocator.blocks[x + 1];
				}
			}
			global_m_allocator.blocks[i].used = false;
			return;
		}
	}
	return;
}
bool strcmp(char far *a, char far *b)
{
	while (*a)
	{
		if (*a != *b)
			return false;
		++a;
		++b;
	}
	return true;
}