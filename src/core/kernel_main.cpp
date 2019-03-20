#include "printf.h"

void _putchar(char character)
{
	asm("hlt");
}

extern "C" void __attribute__((noreturn)) kernel_main()
{
	for (;;)
		asm("hlt");
}

