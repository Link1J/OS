#include <stdint.h>

namespace IDT
{
	struct Registers
	{
		uint64_t ds;
		uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
		uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
		uint64_t intNumber, errorCode;
		uint64_t rip, cs, rflags, userrsp, ss;
    } __attribute__((packed));
	
	typedef void (*InterruptFunction)(Registers* regs);
	
	void Init();
	void SetInterruptHandler(uint8_t interrupt, InterruptFunction function);
}