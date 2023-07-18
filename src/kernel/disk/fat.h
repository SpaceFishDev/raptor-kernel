#pragma once
#include "disk/disk.h"
#include "../../std/stdint.h"

#pragma pack(push, 1)

typedef struct
{
	uint8_t Name[11];
	uint8_t Attributes;
	uint8_t _Reserved;
	uint8_t CreatedTimeTenths;
	uint16_t CreatedTime;
	uint16_t CreatedDate;
	uint16_t AccessedDate;
	uint16_t FirstClusterHigh;
	uint16_t ModifiedTime;
	uint16_t ModifiedDate;
	uint16_t FirstClusterLow;
	uint32_t Size;
} fdir_entry;

#pragma pack(pop)

typedef struct
{
	int Handle;
	bool IsDirectory;
	uint32_t Position;
	uint32_t Size;
} FILE;

enum FAT_Attributes
{
	FAT_ATTRIBUTE_READ_ONLY = 0x01,
	FAT_ATTRIBUTE_HIDDEN = 0x02,
	FAT_ATTRIBUTE_SYSTEM = 0x04,
	FAT_ATTRIBUTE_VOLUME_ID = 0x08,
	FAT_ATTRIBUTE_DIRECTORY = 0x10,
	FAT_ATTRIBUTE_ARCHIVE = 0x20,
	FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN |
						FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

bool init_fat(disk_t *disk);
FILE far *fopen(disk_t *disk, const char *path);
uint32_t fread(disk_t *disk, FILE far *file, uint32_t byteCount,
			   void *dataOut);
bool fread_entry(disk_t *disk, FILE far *file,
				 fdir_entry *dirEntry);
void fclose(FILE far *file);
bool fwrite(disk_t *disk, const char *path, const void *data, uint32_t size); // these two are useless
bool fcreate_entry(disk_t *disk, const char *path, bool isDirectory, const char *name);
