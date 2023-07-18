
#include "disk/fat.h"

#include "../std/memdefs.h"
#include "../std/memory.h"
#include "../std/stddef.h"
#include "../std/stdio.h"
#include "../std/string.h"
#include "../std/utility.h"

#define SECTOR_SIZE 512
#define MAX_PATH_SIZE 256
#define MAX_FILE_HANDLES 10
#define ROOT_DIRECTORY_HANDLE -1

#pragma pack(push, 1)

typedef struct
{
	uint8_t BootJumpInstruction[3];
	uint8_t OemIdentifier[8];
	uint16_t BytesPerSector;
	uint8_t SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t FatCount;
	uint16_t DirEntryCount;
	uint16_t TotalSectors;
	uint8_t MediaDescriptorType;
	uint16_t SectorsPerFat;
	uint16_t SectorsPerTrack;
	uint16_t Heads;
	uint32_t HiddenSectors;
	uint32_t LargeSectorCount;
	uint8_t DriveNumber;
	uint8_t _Reserved;
	uint8_t Signature;
	uint32_t VolumeId;
	uint8_t VolumeLabel[11];
	uint8_t SystemId[8];

} FAT_BootSector;

#pragma pack(pop)

typedef struct
{
	uint8_t Buffer[SECTOR_SIZE];
	FILE Public;
	bool Opened;
	uint32_t FirstCluster;
	uint32_t CurrentCluster;
	uint32_t CurrentSectorInCluster;

} FAT_FileData;

typedef struct
{
	union
	{
		FAT_BootSector BootSector;
		uint8_t BootSectorBytes[SECTOR_SIZE];
	} BS;

	FAT_FileData RootDirectory;

	FAT_FileData OpenedFiles[MAX_FILE_HANDLES];

} FAT_Data;

static FAT_Data far *g_Data;
static uint8_t far *g_Fat = NULL;
static uint32_t g_DataSectionLba;

bool FAT_ReadBootSector(disk_t *disk)
{
	return read_sectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

bool FAT_ReadFat(disk_t *disk)
{
	return read_sectors(disk, g_Data->BS.BootSector.ReservedSectors,
						g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool init_fat(disk_t *disk)
{
	g_Data = (FAT_Data far *)MEMORY_FAT_ADDR;
	if (!FAT_ReadBootSector(disk))
	{
		printf("FAT: read boot sector failed\r\n");
		return false;
	}

	g_Fat = (uint8_t far *)g_Data + sizeof(FAT_Data);
	uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector *
					   g_Data->BS.BootSector.SectorsPerFat;
	if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE)
	{
		printf(
			"FAT: not enough memory to read FAT! Required %lu, only have "
			"%u\r\n",
			sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
		return false;
	}
	if (!FAT_ReadFat(disk))
	{
		printf("FAT: read FAT failed\r\n");
		return false;
	}

	uint32_t rootDirLba =
		g_Data->BS.BootSector.ReservedSectors +
		g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
	uint32_t rootDirSize =
		sizeof(fdir_entry) * g_Data->BS.BootSector.DirEntryCount;

	g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
	g_Data->RootDirectory.Public.IsDirectory = true;
	g_Data->RootDirectory.Public.Position = 0;
	g_Data->RootDirectory.Public.Size =
		sizeof(fdir_entry) * g_Data->BS.BootSector.DirEntryCount;
	g_Data->RootDirectory.Opened = true;
	g_Data->RootDirectory.FirstCluster = rootDirLba;
	g_Data->RootDirectory.CurrentCluster = rootDirLba;
	g_Data->RootDirectory.CurrentSectorInCluster = 0;

	if (!read_sectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer))
	{
		printf("FAT: read root directory failed\r\n");
		return false;
	}

	uint32_t rootDirSectors =
		(rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) /
		g_Data->BS.BootSector.BytesPerSector;

	g_DataSectionLba = rootDirLba + rootDirSectors;
	for (int i = 0; i < MAX_FILE_HANDLES; i++)
	{
		g_Data->OpenedFiles[i].Opened = false;
	}
	return true;
}

uint32_t FAT_ClusterToLba(uint32_t cluster)
{
	return g_DataSectionLba +
		   (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

FILE far *FAT_OpenEntry(disk_t *disk, fdir_entry *entry)
{
	int handle = -1;
	for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++)
	{
		if (!g_Data->OpenedFiles[i].Opened)
			handle = i;
	}
	if (handle < 0)
	{
		printf("FAT: out of file handles\r\n");
		return false;
	}
	FAT_FileData far *fd = &g_Data->OpenedFiles[handle];
	fd->Public.Handle = handle;
	fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
	fd->Public.Position = 0;
	fd->Public.Size = entry->Size;
	fd->FirstCluster =
		entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
	fd->CurrentCluster = fd->FirstCluster;
	fd->CurrentSectorInCluster = 0;
	if (!read_sectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1,
					  fd->Buffer))
	{
		printf("FAT: read error\r\n");
		return false;
	}
	fd->Opened = true;
	return &fd->Public;
}

uint32_t FAT_NextCluster(uint32_t currentCluster)
{
	uint32_t fatIndex = currentCluster * 3 / 2;

	if (currentCluster % 2 == 0)
		return (*(uint16_t far *)(g_Fat + fatIndex)) & 0x0FFF;
	else
		return (*(uint16_t far *)(g_Fat + fatIndex)) >> 4;
}

uint32_t fread(disk_t *disk, FILE far *file, uint32_t byteCount,
			   void *dataOut)
{
	FAT_FileData far *fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
							   ? &g_Data->RootDirectory
							   : &g_Data->OpenedFiles[file->Handle];

	uint8_t *u8DataOut = (uint8_t *)dataOut;
	if (!fd->Public.IsDirectory)
		byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

	while (byteCount > 0)
	{
		uint32_t leftInBuffer =
			SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
		uint32_t take = min(byteCount, leftInBuffer);

		memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
		u8DataOut += take;
		fd->Public.Position += take;
		byteCount -= take;
		if (leftInBuffer == take)
		{
			// Special handling for root directory
			if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE)
			{
				++fd->CurrentCluster;

				// read next sector
				if (!read_sectors(disk, fd->CurrentCluster, 1, fd->Buffer))
				{
					printf("FAT: read error!\r\n");
					break;
				}
			}
			else
			{
				if (++fd->CurrentSectorInCluster >=
					g_Data->BS.BootSector.SectorsPerCluster)
				{
					fd->CurrentSectorInCluster = 0;
					fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
				}
				if (fd->CurrentCluster >= 0xFF8)
				{
					fd->Public.Size = fd->Public.Position;
					break;
				}
				if (!read_sectors(disk,
								  FAT_ClusterToLba(fd->CurrentCluster) +
									  fd->CurrentSectorInCluster,
								  1, fd->Buffer))
				{
					printf("FAT: read error!\r\n");
					break;
				}
			}
		}
	}

	return u8DataOut - (uint8_t *)dataOut;
}

uint32_t FAT_GetUsedClusterCount()
{
	uint32_t usedClusterCount = 0;

	for (uint32_t i = 2; i < g_Data->BS.BootSector.TotalSectors; i++)
	{
		uint32_t fatIndex = i * 3 / 2;
		uint16_t fatEntry;
		if (i % 2 == 0)
		{
			fatEntry = (*(uint16_t far *)(g_Fat + fatIndex)) & 0x0FFF;
		}
		else
		{
			fatEntry = (*(uint16_t far *)(g_Fat + fatIndex)) >> 4;
		}
		if (fatEntry != 0x000 && fatEntry != 0xFFF)
		{
			usedClusterCount++;
		}
	}

	return usedClusterCount;
}
uint32_t FAT_FindFreeCluster()
{
	for (uint32_t i = 2; i < g_Data->BS.BootSector.TotalSectors; i++)
	{
		uint32_t fatIndex = i * 3 / 2;
		uint16_t fatEntry;
		if (i % 2 == 0)
			fatEntry = (*(uint16_t far *)(g_Fat + fatIndex)) & 0x0FFF;
		else
			fatEntry = (*(uint16_t far *)(g_Fat + fatIndex)) >> 4;
		if (fatEntry == 0x000)
			return i;
	}
	return 0;
}

bool fread_entry(disk_t *disk, FILE far *file,
				 fdir_entry *dirEntry)
{
	return fread(disk, file, sizeof(fdir_entry), dirEntry) ==
		   sizeof(fdir_entry);
}

void fclose(FILE far *file)
{
	if (file->Handle == ROOT_DIRECTORY_HANDLE)
	{
		file->Position = 0;
		g_Data->RootDirectory.CurrentCluster =
			g_Data->RootDirectory.FirstCluster;
	}
	else
	{
		g_Data->OpenedFiles[file->Handle].Opened = false;
	}
}

bool FAT_FindFile(disk_t *disk, FILE far *file, const char *name,
				  fdir_entry *entryOut)
{
	char fatName[12];
	fdir_entry entry;
	memset(fatName, ' ', sizeof(fatName));
	fatName[11] = '\0';
	const char *ext = strchr(name, '.');
	if (ext == NULL)
		ext = name + 11;
	for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
		fatName[i] = toupper(name[i]);

	if (ext != NULL)
	{
		for (int i = 0; i < 3 && ext[i + 1]; i++)
			fatName[i + 8] = toupper(ext[i + 1]);
	}

	while (fread_entry(disk, file, &entry))
	{
		if (memcmp(fatName, entry.Name, 11) == 0)
		{
			*entryOut = entry;
			return true;
		}
	}

	return false;
}

FILE far *fopen(disk_t *disk, const char *path)
{
	char name[MAX_PATH_SIZE];
	if (path[0] == '/')
		path++;
	FILE far *current = &g_Data->RootDirectory.Public;
	while (*path)
	{
		// extract next file name from path
		bool isLast = false;
		const char *delim = strchr(path, '/');
		if (delim != NULL)
		{
			memcpy(name, path, delim - path);
			name[delim - path + 1] = '\0';
			path = delim + 1;
		}
		else
		{
			unsigned len = strlen(path);
			memcpy(name, path, len);
			name[len + 1] = '\0';
			path += len;
			isLast = true;
		}
		fdir_entry entry;
		if (FAT_FindFile(disk, current, name, &entry))
		{
			fclose(current);

			// check if directory
			if (!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0)
			{
				printf("FAT: %s not a directory\r\n", name);
				return NULL;
			}

			// open new directory entry
			current = FAT_OpenEntry(disk, &entry);
		}
		else
		{
			fclose(current);

			printf("FAT: %s not found\r\n", name);
			return NULL;
		}
	}

	return current;
}
void writeSector(disk_t *disk, uint32_t lba, const void *data)
{
	write_sectors(disk, lba, 1, (uint8_t far *)data);
}

void writeCluster(disk_t *disk, uint32_t clusterNumber, const void *data)
{
	uint32_t sectorNumber =
		clusterNumber * g_Data->BS.BootSector.SectorsPerCluster +
		g_DataSectionLba;
	write_sectors(disk, sectorNumber, g_Data->BS.BootSector.SectorsPerCluster,
				  (uint8_t far *)data);
}
bool fwrite(disk_t *disk, const char *path, const void *data,
			uint32_t size)
{
	return false;
}
bool fcreate_entry(disk_t *disk, const char *path, bool isDirectory,
				   const char *name)
{
	FILE *parentDir = fopen(disk, path);
	if (!parentDir)
	{
		printf("Failed to open parent directory for creating entry.\n");
		return false;
	}
	fdir_entry entry;
	bool foundFreeEntry = false;
	while (fread_entry(disk, parentDir, &entry))
	{
		if (entry.Name[0] == 0xE5 || entry.Name[0] == 0x00)
		{
			foundFreeEntry = true;
			break;
		}
	}

	if (!foundFreeEntry)
	{
		printf("No free directory entry available for creating a new entry.\n");
		fclose(parentDir);
		return false;
	}
	memset(&entry, 0, sizeof(fdir_entry));
	memcpy(entry.Name, name, 11);
	entry.Attributes =
		isDirectory ? FAT_ATTRIBUTE_DIRECTORY : FAT_ATTRIBUTE_ARCHIVE;
	fread(disk, parentDir, sizeof(fdir_entry),
		  g_Data->RootDirectory.Buffer + parentDir->Position);
	writeSector(disk, FAT_ClusterToLba(g_Data->RootDirectory.CurrentCluster),
				g_Data->RootDirectory.Buffer);

	fclose(parentDir);
	return true;
}
