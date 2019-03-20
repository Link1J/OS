#include "ELF.h"
#include <stdbool.h>

#define ELF_MAGIC						{0x7F,'E','L','F'}
#define ELF_MAGIC_INT 					*(uint32_t*)ELF_MAGIC

//#define ELF_MACHINE_TYPE				0x03	// x86
//#define ELF_MACHINE_TYPE				0x14	// PowerPC
//#define ELF_MACHINE_TYPE				0x08	// MIPS
//#define ELF_MACHINE_TYPE				0x28	// ARM
#define ELF_MACHINE_TYPE				0x3E	// x86_64
//#define ELF_MACHINE_TYPE				0xB7	// AArch64
//#define ELF_MACHINE_TYPE				0xF3	// RISC-V

//#define ELF_CLASS 					0x01	// 32 bit
#define ELF_CLASS						0x02	// 64 bit

#define ELF_ENDIANNESS					0x01	// Little
//#define ELF_ENDIANNESS				0x02	// Big

#define ELF_PROGRAM_HEADER_TYPE_NULL	0x00
#define ELF_PROGRAM_HEADER_TYPE_LOAD	0x01
#define ELF_PROGRAM_HEADER_TYPE_DYNAMIC	0x02
#define ELF_PROGRAM_HEADER_TYPE_INTERP	0x03
#define ELF_PROGRAM_HEADER_TYPE_INTERP	0x04
#define ELF_PROGRAM_HEADER_TYPE_SHLIB	0x05
#define ELF_PROGRAM_HEADER_TYPE_PHDR	0x06


bool validELFFileForPlatfrom(ELF_HEADER* header)
{
	if (header->magic	!= ELF_MAGIC_INT	)
		return false;
	if (header->machine	!= ELF_MACHINE_TYPE	)
		return false;
	if (header->class	!= ELF_CLASS		)
		return false;
	if (header->data	!= ELF_ENDIANNESS	)
		return false;
	
	return true;
}

uint64_t getSize(uint8_t* image)
{
	ELF_HEADER* header = (ELF_HEADER*)image;
	if (!validELFFileForPlatfrom(image))
		return 0;
	
	uint64_t size = 0;
	
	ELF_PROGRAM_HEADER* list = (ELF_PROGRAM_HEADER*)(image + header->program_header_offset);
	
	for (int a = 0; a < header->program_header_entries; a++)
	{
		ELF_PROGRAM_HEADER* curr = list + a;
		if (curr->type == ELF_PROGRAM_HEADER_TYPE_LOAD)
		{ 
			uint64_t offset = curr->virtual_address + curr->memory_size;
			if (offset > size)
				size = offset;
		}
	}
	
	return size;
}