#ifndef __ELF_H__
#define __ELF_H__

#include <stdint.h>
#include <stdbool.h>

typedef unsigned long long	ElfAddr;
typedef unsigned long long	ElfOffs;
typedef unsigned short		ElfHalf;
typedef unsigned int 		ElfWord;
typedef signed int 			ElfSWord;
typedef unsigned long long	ElfXWord;
typedef signed long long	ElfSXWord;
typedef unsigned char		ElfByte;
typedef unsigned short		ElfSection;

struct _ELF_HEADER
{
	ElfWord		magic;
	ElfByte		class;
	ElfByte		data;
	ElfByte		version;
	ElfByte		ABI;
	ElfByte		ABI_version;
	ElfByte		pad[7];
	ElfHalf		type;
	ElfHalf		machine;
	ElfByte		version2;
	ElfAddr		entry_point;
	ElfOffs		program_header_offset;
	ElfOffs		section_header_offset;
	ElfWord		flags;
	ElfHalf		header_size;
	ElfHalf		program_header_size;
	ElfHalf		program_header_entries;
	ElfHalf		section_header_size;
	ElfHalf		section_header_entries;
	ElfHalf		section_strings_entry;
}__attribute__((packed));
typedef struct _ELF_HEADER ELF_HEADER;

#define ELFCLASS32 1
#define ELFCLASS64 2

#define ELFDATALSB 1
#define ELFDATAMSB 2

#define ELFOSABI_SYSV 0
#define ELFOSABI_HPUX 1
#define ELFOSABI_STANDALONE 255

#define ET_NONE 0
#define ET_REL 1
#define ET_EXEC 2
#define ET_DYN 3
#define ET_CORE 4

#define EMT_X86_64 0x3E
#define EMT_IA64 0x32

struct _ELF_SECTION_HEADER
{
	ElfWord		name;
	ElfWord		type;
	ElfXWord	flags;
	ElfAddr		virtual_address;
	ElfOffs		offset;
	ElfXWord	size;
	ElfWord		link;
	ElfWord		info;
	ElfXWord	address_align;
	ElfXWord	entry_size;
}__attribute__((packed));
typedef struct _ELF_SECTION_HEADER ELF_SECTION_HEADER;

#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL 9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11

#define SHF_WRITE 0x01
#define SHF_ALLOC 0x02
#define SHF_EXECINSTR 0x04

struct _ELF_PROGRAM_HEADER
{
	ElfWord		type;
	ElfWord		flags;
	ElfOffs		offset;
	ElfAddr		virtual_address;
	ElfAddr		physical_address;
	ElfXWord	file_size;
	ElfXWord	memory_size;
	ElfXWord	align;
}__attribute__((packed));
typedef struct _ELF_PROGRAM_HEADER ELF_PROGRAM_HEADER;

#define PT_NULL 0
#define PT_LOAD 1
#define PT_DYNAMIC 2
#define PT_INTERP 3
#define PT_NOTE 4

#define PF_EXEC 1
#define PF_WRITABLE 2
#define PF_READABLE 4

struct _ELF_DYNAMIC_ENTRY {
    ElfSXWord	tag;
    ElfXWord	value;
}__attribute__((packed));
typedef struct _ELF_DYNAMIC_ENTRY ELF_DYNAMIC_ENTRY;

#define DT_NULL 0
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA 7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT 12
#define DT_FINI 13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_REL 17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_JMPREL 23
#define DT_INIT_ARRAY 0x19
#define DT_INIT_ARRAYSZ 0x1B

struct _ELF_SYMBOL {
    ElfWord		symbolNameOffset;
    ElfByte		info;
    ElfByte		reserved;
    ElfHalf		sectionTableIndex;
    ElfAddr		value;
    ElfXWord	size;
}__attribute__((packed));
typedef struct _ELF_SYMBOL ELF_SYMBOL;

#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2

#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4

struct _ELF_REL_A {
    ElfAddr		address;
    ElfXWord	info;
    ElfSXWord	addend;
}__attribute__((packed));
typedef struct _ELF_REL_A ELF_REL_A;

#define R_NONE 0
#define R_64 1
#define R_PC32 2
#define R_GOT32 3
#define R_PLT32 4
#define R_COPY 5
#define R_GLOB_DAT 6
#define R_JUMP_SLOT 7
#define R_RELATIVE 8
#define R_GOTPCREL 9
#define R_32 10
#define R_32S 11
#define R_16 12
#define R_PC16 13
#define R_8 14
#define R_PC8 15
#define R_DTPMOD64 16
#define R_DTPOFF64 17
#define R_TPOFF64 18
#define R_TLSGD 19
#define R_TLSLD 20
#define R_DTPOFF32 21
#define R_GOTTPOFF 22
#define R_TPOFF32 23
#define R_PC64 24
#define R_GOTOFF64 25
#define R_GOTPC32 26
#define R_SIZE32 32
#define R_SIZE64 33
#define R_GOTPC32_TLSDESC 34
#define R_TLSDESC_CALL 35
#define R_TLSDESC 36
#define R_IRELATIVE 37

uint64_t getELFSize(const uint8_t* image);
uint64_t getELFTextAddr(const uint8_t* image, const uint8_t* processImg);
bool     prepareELF(const uint8_t* diskImg, uint8_t* processImg, ElfAddr* entryPoint);

#endif