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

uint64_t getELFSize(const uint8_t* image)
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

uint64_t getELFTextAddr(const uint8_t* image, const uint8_t* processImg)
{
    ELF_HEADER* header = (ELF_HEADER*)image;

    ELF_SECTION_HEADER* shList = (ELF_SECTION_HEADER*)(image + header->section_header_offset);
    char* sectionNameTable = (char*)((uint64_t)image + shList[header->section_strings_entry].offset);
    for(int s = 0; s < header->section_header_entries; s++) {
        ELF_SECTION_HEADER* section = &shList[s];
        if(section->type == SHT_PROGBITS) {
            char* name = &sectionNameTable[section->name];
            if(strcmp(name, ".text")) {
                return (uint64_t)processImg + section->virtual_address;
            }
        }
    }

    return 0;
}

bool prepareELF(const uint8_t* diskImg, uint8_t* processImg, ElfAddr* entryPoint)
{
    ELF_HEADER* header = (ELF_HEADER*)diskImg;

    ELF_PROGRAM_HEADER* segList = (ELF_PROGRAM_HEADER*)(diskImg + header->program_header_offset);
    for(int s = 0; s < header->program_header_entries; s++) {
        ELF_PROGRAM_HEADER* segment = &segList[s];
        if(segment->type == PT_LOAD) 
		{
            const uint8_t* src = diskImg + segment->offset;
            uint8_t* dest = processImg + segment->virtual_address;

            // Load the data into the segment
            for(uint64_t i = 0; i < segment->file_size; i++)
                dest[i] = src[i];
            
            // fill the rest of the segment with zeros
            for(uint64_t i = segment->file_size; i < segment->memory_size; i++)
                dest[i] = 0;
        }
		else if(segment->type == PT_DYNAMIC) 
		{
            ElfAddr symtabAddr;

            ElfAddr relaAddr = 0;
            ElfXWord relaSize = 0;
            ElfXWord relaEntrySize = sizeof(ELF_REL_A);
            ElfXWord relaCount = 0;

            ElfAddr pltrelAddr = 0;
            ElfXWord pltrelSize = 0;
            ElfXWord pltrelEntrySize = sizeof(ELF_REL_A);
            ElfXWord pltrelCount = 0;

            ElfAddr initArrayAddr = 0;
            ElfXWord initArraySz = 0;

            ELF_DYNAMIC_ENTRY* dyn = (ELF_DYNAMIC_ENTRY*)(processImg + segment->virtual_address);
            while(dyn->tag != DT_NULL) {
                if(dyn->tag == DT_SYMTAB) {
                    symtabAddr = dyn->value;
                }
                else if(dyn->tag == DT_JMPREL) {
                    pltrelAddr = dyn->value;
                }
                else if(dyn->tag == DT_PLTRELSZ) {
                    pltrelSize = dyn->value;
                }
                else if(dyn->tag == DT_RELAENT) {
                    pltrelEntrySize = dyn->value;
                }
                else if(dyn->tag == DT_RELA) {
                    relaAddr = dyn->value;
                }
                else if(dyn->tag == DT_RELASZ) {
                    relaSize = dyn->value;
                }
                else if(dyn->tag == DT_RELAENT) {
                    relaEntrySize = dyn->value;
                } else if(dyn->tag == DT_INIT_ARRAY) {
                    initArrayAddr = dyn->value;
                } else if(dyn->tag == DT_INIT_ARRAYSZ) {
                    initArraySz = dyn->value;
                }

                dyn++;
            }

            relaCount = relaSize / relaEntrySize;
            pltrelCount = pltrelSize / pltrelEntrySize;

            ELF_REL_A* relaList = (ELF_REL_A*)(processImg + relaAddr);
            ELF_REL_A* pltrelList = (ELF_REL_A*)(processImg + pltrelAddr);
            ELF_SYMBOL* symList = (ELF_SYMBOL*)(processImg + symtabAddr);

            for(unsigned int i = 0; i < relaCount; i++) {
                ELF_REL_A* rel = &relaList[i];

                unsigned int symIndex = rel->info >> 32;
                unsigned int relType = rel->info & 0xFFFFFFFF;
                ElfAddr target = (ElfAddr)processImg + rel->address;
                ElfXWord addend = rel->addend;

                ElfAddr symAddr = (ElfAddr)processImg + symList[symIndex].value;
                ElfXWord finalAddend = 0;
                int size = 0;

                switch(relType) {
                case R_NONE: 
                case R_COPY: continue;
                case R_64:      size = 8; finalAddend = symAddr + addend; break;
                case R_PC32:    size = 4; finalAddend = symAddr + addend - target; break;
                case R_GLOB_DAT:
                case R_JUMP_SLOT: size = 8; finalAddend = symAddr; break;
                case R_RELATIVE: size = 8; finalAddend = addend + (ElfXWord)processImg; break;
                case R_32S:
                case R_32: size = 4; finalAddend = symAddr + addend; break;
                case R_PC64: size = 8; finalAddend = symAddr + addend - target; break;
                case R_SIZE32: size = 4; finalAddend = symList[symIndex].size + addend; break;
                case R_SIZE64: size = 8; finalAddend = symList[symIndex].size + addend; break;
                default: return false;
                }

                switch(size) {
                case 1: *(unsigned char*)target = finalAddend; break;
                case 2: *(unsigned short*)target = finalAddend; break;
                case 4: *(unsigned int*)target = finalAddend; break;
                case 8: *(unsigned long long*)target = finalAddend; break;
                default: break;
                }
            }

            for(unsigned int i = 0; i < pltrelCount; i++) {
                ELF_REL_A* rel = &pltrelList[i];

                unsigned int symIndex = rel->info >> 32;
                unsigned int relType = rel->info & 0xFFFFFFFF;
                ElfAddr target = (ElfAddr)processImg + rel->address;
                ElfXWord addend = rel->addend;

                ElfAddr symAddr = (ElfAddr)processImg + symList[symIndex].value;
                ElfXWord finalAddend = 0;
                int size = 0;

                switch(relType) {
                case R_NONE: 
                case R_COPY: continue;
                case R_64:      size = 8; finalAddend = symAddr + addend; break;
                case R_PC32:    size = 4; finalAddend = symAddr + addend - target; break;
                case R_GLOB_DAT:
                case R_JUMP_SLOT: size = 8; finalAddend = symAddr; break;
                case R_RELATIVE: size = 8; finalAddend = addend + (ElfXWord)processImg; break;
                case R_32S:
                case R_32: size = 4; finalAddend = symAddr + addend; break;
                case R_PC64: size = 8; finalAddend = symAddr + addend - target; break;
                case R_SIZE32: size = 4; finalAddend = symList[symIndex].size + addend; break;
                case R_SIZE64: size = 8; finalAddend = symList[symIndex].size + addend; break;
                default: return false;
                }

                switch(size) {
                case 1: *(unsigned char*)target = finalAddend; break;
                case 2: *(unsigned short*)target = finalAddend; break;
                case 4: *(unsigned int*)target = finalAddend; break;
                case 8: *(unsigned long long*)target = finalAddend; break;
                default: break;
                }
            }

            if(initArrayAddr != 0)
            {
                initArrayAddr += (uint64_t)processImg;

                typedef void (**INITARRAY)();
                INITARRAY arr = (INITARRAY)initArrayAddr;

                for(int i = 0; i < initArraySz / 8; i++)
                    arr[i]();
            }
        }
    }

    *entryPoint = (ElfAddr)processImg + header->entry_point;
    return true;
}
