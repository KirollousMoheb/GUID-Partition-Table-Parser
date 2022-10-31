#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
typedef struct
{
  uint8_t status;
  uint8_t first_chs[3];
  uint8_t partition_type;
  uint8_t last_chs[3];
  uint32_t lba;
  uint32_t sector_count;

} MBR_PartitionEntry;
typedef struct
{
  uint32_t part_type_guid1;
  uint16_t part_type_guid2;
  uint16_t part_type_guid3;
  uint16_t part_type_guid4;
  uint16_t part_type_guid5;
  uint32_t part_type_guid6;

  uint32_t unique_part_guid1;
  uint16_t unique_part_guid2;
  uint16_t unique_part_guid3;
  uint16_t unique_part_guid4;
  uint16_t unique_part_guid5;
  uint32_t unique_part_guid6;

  uint64_t first_lba;
  uint64_t last_lba;
  uint64_t attr_flags;
  uint8_t part_name[72];

} GPT_PartitionEntry;
typedef struct
{

  char signature[8];
  uint32_t revision;
  uint32_t header_size;
  uint32_t crc32;
  uint32_t reserved;
  uint64_t current_lba;
  uint64_t backup_lba;
  uint64_t first_usable_lba;
  uint64_t last_usable_lba;
  uint32_t disk_guid1;
  uint16_t disk_guid2;
  uint16_t disk_guid3;
  uint16_t disk_guid4;
  uint16_t disk_guid5;
  uint32_t disk_guid6;
  uint64_t part_entry_start_lba;
  uint32_t num_part_entries;
  uint32_t part_entry_size;
  uint32_t crc32_part_array;

} GPT_HEADER_FORMAT;

uint16_t
swap_uint16 (uint16_t val)
{
  return (val << 8) | (val >> 8);
}

uint64_t
swap_uint64 (uint64_t val)
{
  val =
    ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) &
					    0x00FF00FF00FF00FFULL);
  val =
    ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) &
					     0x0000FFFF0000FFFFULL);
  return (val << 32) | (val >> 32);
}

uint32_t
swap_uint32 (uint32_t val)
{
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
  return (val << 16) | (val >> 16);
}

char *
convertSize (uint64_t bytes)
{
  static char str[20];

  long gb = 1024 * 1024 * 1024;
  long mb = 1024 * 1024;
  long kb = 1024;
  if (bytes >= gb)
    {
      sprintf (str, "%0.1fG", (float) bytes / gb);
    }
  else if (bytes >= mb)
    {
      sprintf (str, "%0.1fM", (float) bytes / mb);
    }
  else if (bytes >= kb)
    {
      sprintf (str, "%0.1fK", (float) bytes / kb);
    }
  else
    {
      sprintf (str, "%ldB", bytes);
    }
  return str;
}

int
main (int argc, char **argv)
{

  //printf("size:%ld",sizeof(GPT_PartitionEntry));
	if(argc<2){
	printf("Please Enter Filename\n");
	return -1;
	}
  char buf[96];
  char buf2[550];
  char str[33];
  int fd = open (argv[1], O_RDONLY);
  if(fd==-1){
  printf("Can't open input file");
  return -2;
  }
  int partition_count = 1;
  lseek (fd, 1 * 512, SEEK_SET);
  read (fd, buf, 96);
  GPT_HEADER_FORMAT *gptheaderptr = (GPT_HEADER_FORMAT *) & buf[0];
  GPT_PartitionEntry *gptpartptr = (GPT_PartitionEntry *) & buf2[0];
  printf
    ("=================================================================\n");
  printf ("Signature: %s\n", gptheaderptr[0].signature);
  printf ("Revision: %08x\n", swap_uint32 (gptheaderptr[0].revision));
  printf ("Header Size: %d\n", (gptheaderptr[0].header_size));
  printf ("Header CRC32: %d\n", (gptheaderptr[0].crc32));
  printf ("Reserved : %d\n", (gptheaderptr[0].reserved));
  printf ("Current LBA : %ld\n", (gptheaderptr[0].current_lba));
  printf ("Backup LBA : %ld\n", (gptheaderptr[0].backup_lba));
  printf ("First Usable LBA : %ld\n", (gptheaderptr[0].first_usable_lba));
  printf ("Last Usable LBA : %ld\n", (gptheaderptr[0].last_usable_lba));
  printf ("UID : %X-%X-%X-%X-%X%X\n", (gptheaderptr[0].disk_guid1),
	  (gptheaderptr[0].disk_guid2), (gptheaderptr[0].disk_guid3),
	  swap_uint16 (gptheaderptr[0].disk_guid4),
	  swap_uint16 (gptheaderptr[0].disk_guid5),
	  swap_uint32 (gptheaderptr[0].disk_guid6));
  printf ("Parition Entry Start LBA : %ld\n",
	  (gptheaderptr[0].part_entry_start_lba));
  printf ("Number of Partition Entries : %d\n",
	  (gptheaderptr[0].num_part_entries));
  printf ("Partition Entry Size : %d\n", (gptheaderptr[0].part_entry_size));
  printf ("CRC Partition Array : %d\n\n", (gptheaderptr[0].crc32_part_array));

  lseek (fd, 2 * 512, SEEK_SET);
  read (fd, buf2, 550);

  printf
    ("=================================================================\n");
  printf ("%-10s       %-10s %-10s %-10s  %-10s \n", "Device",
	  "Start", "End", "Sectors", "  Size");
  printf
    ("=================================================================\n");
  int current_lba = 3, z = 0;
  while (current_lba != 33)
    {
      for (int i = 0; i < 4; i++)
	{
	  if ((gptpartptr[i].last_lba) == 0)
	    {
	      break;
	    }
	  printf
	    ("%s%-5d    %-10lu %-10lu %-12lu %s \n",
	     argv[1], partition_count++,
	     (gptpartptr[i].first_lba),
	     (gptpartptr[i].last_lba),
	     (gptpartptr[i].last_lba) - (gptpartptr[i].first_lba) + 1,
	     convertSize (((gptpartptr[i].last_lba) -
			   (gptpartptr[i].first_lba)) * 512));
	}
      lseek (fd, (current_lba++) * 512, SEEK_SET);
      read (fd, buf2, 550);
    }

  return 0;
}
