#include "printf.h"

namespace Error
{
	void Panic(const char* message)
	{
		printf("KERNEL PANIC: %s", message);
		asm("hlt");
	}
}