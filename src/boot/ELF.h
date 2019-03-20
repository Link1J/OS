#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>

struct _ELF_HEADER
{
	uint32_t  magic;
	uint8_t   class;
	uint8_t   data;
	uint8_t   version;
	uint8_t   ABI;
	uint8_t   ABI_version;
	uint8_t   pad[7];
	uint16_t  type;
	uint16_t  machine;
	uint8_t   version2;
	uintptr_t entry_point;
	uintptr_t program_header_offset;
	uintptr_t section_header_offset;
	uint32_t  flags;
	uint16_t  header_size;
	uint16_t  program_header_size;
	uint16_t  program_header_entries;
	uint16_t  section_header_size;
	uint16_t  section_header_entries;
	uint16_t  section_strings_entry;
}__attribute__((packed));
typedef struct _ELF_HEADER ELF_HEADER;

struct _ELF_PROGRAM_HEADER
{
	uint32_t  type;
	uint8_t   flags;
	uintptr_t offset;
	uintptr_t virtual_address;
	uintptr_t physical_address;
	uintptr_t file_size;
	uintptr_t memory_size;
	uintptr_t align;
	uint8_t   pad[sizeof(uintptr_t)];
}__attribute__((packed));
typedef struct _ELF_PROGRAM_HEADER ELF_PROGRAM_HEADER;

struct _ELF_SECTION_HEADER
{
	uint32_t  name;
	uint32_t  type;
	uint8_t   flags;
	uintptr_t virtual_address;
	uintptr_t offset;
	uintptr_t size;
	uint32_t  link;
	uint32_t  info;
	uintptr_t address_align;
	uintptr_t entry_size;
	uint8_t   pad[sizeof(uintptr_t)];
}__attribute__((packed));
typedef struct _ELF_SECTION_HEADER ELF_SECTION_HEADER;


uint64_t getSize(uint8_t* image);



#endif