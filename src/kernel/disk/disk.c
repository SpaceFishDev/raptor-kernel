#include "disk/disk.h"

#include "../std/stddef.h"
#include "../std/stdint.h"
#include "../std/stdio.h"
#include "x86.h"

bool init_disk(disk_t *disk, uint8_t drive_number)
{
	uint8_t driveType;
	uint16_t cyllinders, sectors, heads;

	if (!x86_Disk_GetDriveParams(disk->id, &driveType, &cyllinders, &sectors,
								 &heads))
	{
		return false;
	}
	disk->id = drive_number;
	disk->cyllinders = cyllinders + 1;
	disk->heads = heads + 1;
	disk->sectors = sectors;
	return true;
}

void LBA_TO_CHS(disk_t *disk, uint32_t lba, uint16_t *cyllinders,
				uint16_t *sectors, uint16_t *head)
{
	*sectors = lba % disk->sectors + 1;
	*cyllinders = (lba / disk->sectors) / disk->heads;
	*head = (lba / disk->sectors) % disk->heads;
}

bool read_sectors(disk_t *disk, uint32_t lba, uint8_t sectors,
				  uint8_t far *data)
{
	uint16_t cyllinder, head, sector;
	LBA_TO_CHS(disk, lba, &cyllinder, &sector, &head);
	for (int i = 0; i < 3; i++)
	{
		if (x86_Disk_Read(disk->id, cyllinder, sector, head, sectors, data))
		{
			return true;
		}
		x86_Disk_Reset(disk->id);
	}

	return false;
}

bool write_sectors(disk_t *disk, uint32_t lba, uint8_t sectors,
				   uint8_t far *data)
{
	uint16_t cyllinder, head, sector;

	LBA_TO_CHS(disk, lba, &cyllinder, &sector, &head);

	for (int i = 0; i < 3; i++)
	{
		if (x86_Disk_Write_Sector(disk->id, cyllinder, sector, head, sectors,
								  data))
		{
			return true;
		}
		x86_Disk_Reset(disk->id);
	}
	return false;
}
