#include "GDT.hpp"
#include "MemoryManager.hpp"

#include "printf.h"
#include <stdint.h>

namespace IDT {
    extern uint8_t interruptStack[4096];
}

namespace GDT
{
	struct GDTEntry
    {
        uint16_t	limit1;
        uint16_t	base1;
        uint8_t		base2;
        uint8_t		access;
        uint8_t		limit2flags;
        uint8_t		base3;
    } __attribute__((packed));

    struct TSSDesc
    {
        uint16_t	limit1;
        uint16_t	base1;
        uint8_t		base2;
        uint8_t		access;
        uint8_t		limit2flags;
        uint8_t		base3;
        uint32_t	base4;
        uint32_t	reserved;
    } __attribute__((packed));

    struct TSS
    {
        uint32_t	reserved;
        uint64_t	rsp0;
        uint64_t	rsp1;
        uint64_t	rsp2;
        uint64_t	reserved2;
        uint64_t	ist1;
        uint64_t	ist2;
        uint64_t	ist3;
        uint64_t	ist4;
        uint64_t	ist5;
        uint64_t	ist6;
        uint64_t	ist7;
        uint64_t	reserved3;
        uint16_t	unused;
        uint16_t	iopbOffset;
    } __attribute__((packed));

    struct GDTDesc
    {
        uint16_t	size;
        uint64_t	offset;
    } __attribute__((packed));

    static GDTEntry gdt[7];
    static TSS tss;
	static GDTDesc desc;

	void Init(uint64_t kernelStart)
	{
		printf("Initializing GDT\n");

        // null descriptor
        gdt[0] = { 0 };

        // kernel code
        gdt[1].limit1 = 0xFFFF;
        gdt[1].base1 = 0x0000;
        gdt[1].base2 = 0x00;
        gdt[1].access = 0b10011010;
        gdt[1].limit2flags = 0b10101111;
        gdt[1].base3 = 0x00;

        // kernel data
        gdt[2].limit1 = 0xFFFF;
        gdt[2].base1 = 0x0000;
        gdt[2].base2 = 0x00;
        gdt[2].access = 0b10010010;
        gdt[2].limit2flags = 0b10101111;
        gdt[2].base3 = 0x00;

        // user code
        gdt[3].limit1 = 0xFFFF;
        gdt[3].base1 = 0x0000;
        gdt[3].base2 = 0x00;
        gdt[3].access = 0b11111010;
        gdt[3].limit2flags = 0b10101111;
        gdt[3].base3 = 0x00;

        // user data
        gdt[4].limit1 = 0xFFFF;
        gdt[4].base1 = 0x0000;
        gdt[4].base2 = 0x00;
        gdt[4].access = 0b11110010;
        gdt[4].limit2flags = 0b10101111;
        gdt[4].base3 = 0x00;

        // TSS, needed for inter priviledge level interrupts 
        tss = { 0 };
        tss.iopbOffset = sizeof(tss);
        // this is the stack pointer that will be loaded when an interrupt CHANGES the priviledge level to 0
        tss.rsp0 = (uint64_t)IDT::interruptStack + sizeof(IDT::interruptStack);
		tss.ist1 = (uint64_t)IDT::interruptStack + sizeof(IDT::interruptStack);

        TSSDesc* tssDesc = (TSSDesc*)&gdt[5];
        *tssDesc = { 0 };
        tssDesc->limit1 		=  (sizeof(tss) - 1)		& 0xFFFF;
        tssDesc->limit2flags	= ((sizeof(tss) - 1) >> 16)	& 0xF;
        tssDesc->base1 			=  (uint64_t)&tss			& 0xFFFF;
        tssDesc->base2 			= ((uint64_t)&tss >> 16)	& 0xFF;
        tssDesc->base3 			= ((uint64_t)&tss >> 24)	& 0xFF;
        tssDesc->base4 			= ((uint64_t)&tss >> 32)	& 0xFFFFFFFF;
        tssDesc->access 		= 0b11101001;

        desc = { sizeof(gdt) - 1, (uint64_t)(gdt) };
        __asm__ __volatile__ (
			"lgdtq	%0;"                    // tell cpu to use new GDT
			"lea 	0x8, %%rax;"
			"lea	flush, %%rbx;"
			"add	%%rcx, %%rbx;"
			"push	%%rax;"
			"push	%%rbx;"
			"lea 	0x10, %%rax;"             // kernel data selector
            "mov	%%ax, %%ds;"
            "mov	%%ax, %%es;"
            "mov	%%ax, %%fs;"
            "mov	%%ax, %%gs;"
            "mov	%%ax, %%ss;"
			"lretq;"
			"flush:"
            : 
            : "m" (desc), "c"(kernelStart)
            : "rax", "rbx"
        );
		
        __asm__ __volatile__ (
            "ltr %%ax"              // tell cpu to use new TSS
            : : "a" (5 * 8)
        );
		
		printf("GDT Setup Completed\n");
	}
}