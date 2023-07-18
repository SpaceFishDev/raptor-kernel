#ifndef DISK_H
#define DISK_H

#include "../std/stddef.h"
#include "../../std/stdint.h"

typedef struct
{
	uint8_t id;
	uint16_t cyllinders;
	uint16_t sectors;
	uint16_t heads;

} disk_t;

bool init_disk(disk_t *disk, uint8_t drive_number);
bool read_sectors(disk_t *disk, uint32_t lba, uint8_t sectors,
				  uint8_t far *data);
bool write_sectors(disk_t *disk, uint32_t lba, uint8_t sectors,
				   uint8_t far *data);

#endif
