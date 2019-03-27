#include "IDT.hpp"
#include "MemoryUtils.hpp"
#include "Error.hpp"
#include "Screen.hpp"

#include "printf.h"
#include <stdint.h>
#include <string.h>

#define ISRSTUB(vectno) extern "C" void ISRSTUB_##vectno();
#define ISRSTUBE(vectno) extern "C" void ISRSTUB_##vectno();
#include "ISR.inc"

namespace IDT {
    uint8_t interruptStack[4096];

	constexpr uint8_t ExceptionDiv0 = 0;
    constexpr uint8_t ExceptionDebug = 1;
    constexpr uint8_t ExceptionNMI = 2;
    constexpr uint8_t ExceptionBreakpoint = 3;
    constexpr uint8_t ExceptionOverflow = 4;
    constexpr uint8_t ExceptionBoundRangeExceeded = 5;
    constexpr uint8_t ExceptionInvalidOpcode = 6;
    constexpr uint8_t ExceptionDeviceUnavailable = 7;
    constexpr uint8_t ExceptionDoubleFault = 8;
    constexpr uint8_t ExceptionCoprocesssorSegmentOverrun = 9;
    constexpr uint8_t ExceptionInvalidTSS = 10;
    constexpr uint8_t ExceptionSegmentNotPresent = 11;
    constexpr uint8_t ExceptionStackSegmentNotPresent = 12;
    constexpr uint8_t ExceptionGPFault = 13;
    constexpr uint8_t ExceptionPageFault = 14;
    constexpr uint8_t ExceptionFPException = 16;
    constexpr uint8_t ExceptionAlignmentCheck = 17;
    constexpr uint8_t ExceptionMachineCheck = 18;
    constexpr uint8_t ExceptionSIMDFP = 19;
    constexpr uint8_t ExceptionVirtualization = 20;
    constexpr uint8_t ExceptionSecurity = 30;

    constexpr uint8_t APICTimer = 100;
    constexpr uint8_t APICError = 101;
    
    constexpr uint8_t APICSpurious = 255;

	struct IDTEntry 
	{
		uint16_t offset1;   // offset bits 0..15
		uint16_t selector;  // a code segment selector in GDT or LDT
		uint8_t  ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
		uint8_t  type_attr; // type and attributes
		uint16_t offset2;   // offset bits 16..31
		uint32_t offset3;   // offset bits 32..63
		uint32_t zero;      // reserved
	} __attribute__((packed));

    struct IDTDesc
    {
        uint16_t	size;
        uint64_t	offset;
    } __attribute__((packed));
	
	static IDTEntry 			idt[256];
    static IDTDesc 				idtDesc;
	static InterruptFunction	fuctions[256];
	static uint64_t				kernelStart;
	
	void SetInterrupt(uint8_t number, void (*function)())
	{
		idt[number].offset1 = ((uint64_t)function >>  0) & 0xFFFF    ;
		idt[number].offset2 = ((uint64_t)function >> 16) & 0xFFFF    ;
		idt[number].offset3 = ((uint64_t)function >> 32) & 0xFFFFFFFF;
		
		idt[number].selector = 0x8;
		idt[number].zero = 0;
		idt[number].ist = 0x1;

		idt[number].type_attr = (1 << 7) | (3 << 5) | 0xE; 
	}
	
	extern "C" void InterruptFunctionCaller(Registers* regs)
    {
        if(fuctions[regs->intNumber] == nullptr)
            printf("INVALID INTERRUPT: %i\n", regs->intNumber);
        else
            fuctions[regs->intNumber](regs);
    }
	
	static void Interrupt_Exceptions_Handler(Registers* regs)
    {
		const char* errorMsg;
		int status;
		const char* statusMsg;
		
		uint64_t cr2;
		asm volatile (
			"movq %%cr2, %0"
            : "=r"(cr2)
        );
		
        switch (regs->intNumber)
        {
		default:									errorMsg = "No message";					status = 0;	break;
		case ExceptionDiv0:							errorMsg = "Divide by zero error";			status = 3;	break;
        case ExceptionDebug:						errorMsg = "Debug trap";					status = 2;	break;
        case ExceptionNMI:							errorMsg = "Non maskable interrupt";		status = 1;	break;
        case ExceptionBreakpoint:					errorMsg = "Breakpoint";					status = 3;	break;
        case ExceptionOverflow:						errorMsg = "Overflow";						status = 3;	break;
        case ExceptionBoundRangeExceeded:			errorMsg = "Bound Range exceeded";			status = 3;	break;
        case ExceptionInvalidOpcode: 				errorMsg = "Invalid opcode";				status = 3;	break;
        case ExceptionDeviceUnavailable: 			errorMsg = "Device unavailable";			status = 3;	break;
        case ExceptionDoubleFault: 					errorMsg = "Double fault";					status = 0;	break;
        case ExceptionCoprocesssorSegmentOverrun: 	errorMsg = "Coprocessor error";				status = 3;	break;
        case ExceptionInvalidTSS:					errorMsg = "Invalid TSS";					status = 0;	break;
        case ExceptionSegmentNotPresent:			errorMsg = "Segment not present";			status = 0;	break;
        case ExceptionStackSegmentNotPresent:		errorMsg = "Stack segment not present";		status = 0;	break;
        case ExceptionGPFault:						errorMsg = "General protection fault";		status = 0;	break;
        case ExceptionPageFault:					errorMsg = "Page fault";					status = 0;	break;
        case ExceptionFPException:					errorMsg = "Floating point exception";		status = 3;	break;
        case ExceptionAlignmentCheck:				errorMsg = "Alignment check";				status = 3;	break;
        case ExceptionMachineCheck:					errorMsg = "Machine check";					status = 3;	break;
        case ExceptionSIMDFP:						errorMsg = "SIMD floating point exception";	status = 3;	break;
        case ExceptionVirtualization:				errorMsg = "Virtualization exception";		status = 3;	break;
        case ExceptionSecurity:						errorMsg = "Security exception";			status = 0;	break;
        }

		switch (status)
        {
		case 0: statusMsg = "ABORT";		break;
		case 1: statusMsg = "Interrupt";	break;
		case 2: statusMsg = "Trap";			break;
		case 3: statusMsg = "Fault";		break;
		}
		
		printf("INT %d %-29s Status Code: %-9s\n", regs->intNumber, errorMsg, statusMsg);
		printf("RIP: %016llX (%016llX)      | RSP: %016llX\n", regs->rip, regs->rip - kernelStart, regs->userrsp);
		printf("RAX: %016llX | RCX: %016llX | RDX: %016llX\n", regs->rax, regs->rcx, regs->rdx);
		printf("RBX: %016llX | RBP: %016llX | RSI: %016llX\n", regs->rbx, regs->rbp, regs->rsi);
		printf("RDI: %016llX | R8 : %016llX | R9 : %016llX\n", regs->rdi, regs->r8, regs->r9);
		printf("R10: %016llX | R11: %016llX | R12: %016llX\n", regs->r10, regs->r11, regs->r12);
		printf("R13: %016llX | R14: %016llX | R15: %016llX\n", regs->r13, regs->r14, regs->r15);
		
		if (regs->intNumber == ExceptionPageFault)
		{
			printf("%sPresent, %s, User: %d, Reserved write: %d%s\n",
				(regs->errorCode >> 0) & 0b1
					? ""
					: "Not "
				,
				(regs->errorCode >> 1) & 0b1
					? "Write"
					: "Read"
				,
				(regs->errorCode >> 2) & 0b1
				, 
				(regs->errorCode >> 3) & 0b1
				,
				(regs->errorCode >> 4) & 0b1
					? ", Instruction Fetch"
					: ""
				);
			
			if (!((regs->errorCode >> 4) & 0b1))
				printf("Virtual Address: %016llX\n", cr2);
		}
		
		
		if (status == 0)
			asm("hlt");
    }


	void EnableInterrupts () { asm volatile ("sti"); }
    void DisableInterrupts() { asm volatile ("cli"); }
	
	void Init(uint64_t kernelStartIn)
	{
		printf("Initializing IDT\n");

		kernelStart = kernelStartIn;		
		memset(idt, 0, sizeof(idt));
		
		idtDesc.offset = (uint64_t)idt;
		idtDesc.size = sizeof(idt) - 1;
		
		#undef ISRSTUB
		#undef ISRSTUBE
		#define ISRSTUB(vectno) SetInterrupt(vectno, ISRSTUB_##vectno);
		#define ISRSTUBE(vectno) SetInterrupt(vectno, ISRSTUB_##vectno);
		#include "ISR.inc"

		for (int a = 0; a < 32; a++)
			SetInterruptHandler(a, Interrupt_Exceptions_Handler);
		
		DisableInterrupts();
		__asm__ __volatile__ (
            "lidtq %0;"               // tell cpu to use new IDT
			: 
			: "m"(idtDesc)
			);
		
		printf("IDT Setup Completed\n");
	}
	
	void SetInterruptHandler(uint8_t interrupt, InterruptFunction function)
	{
		fuctions[interrupt] = function;
	}
}