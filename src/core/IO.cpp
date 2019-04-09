#include "IO.hpp"

namespace IO
{
	namespace In
	{
		uint8_t Byte(uint16_t port)
		{
			uint8_t value;
			asm volatile("inb %1, %0"
						: "=a"(value)
						: "Nd"(port) 
						);
			return value;
		}
		
		uint16_t Word(uint16_t port)
		{
			uint16_t value;
			asm volatile("inw %1, %0"
						: "=a"(value)
						: "Nd"(port) 
						);
			return value;
		}
		
		uint32_t Dword(uint16_t port)
		{
			uint32_t value;
			asm volatile("inl %1, %0"
						: "=a"(value)
						: "Nd"(port) 
						);
			return value;
		}
	}

	namespace InString
	{
		void Word(uint16_t port, uintptr_t addr, uint32_t cnt)
		{
			asm volatile("cld; rep insw" 
						: "=D" (addr), "=c" (cnt) 
						: "d" (port), "0" (addr), "1" (cnt) 
						: "memory", "cc");
		}

		void Dword(uint16_t port, uintptr_t addr, uint32_t cnt)
		{
			asm volatile("cld; rep insl" 
						: "=D" (addr), "=c" (cnt) 
						: "d" (port), "0" (addr), "1" (cnt) 
						: "memory", "cc");
		}
	}

	namespace Out
	{
		void Byte(uint16_t port, uint8_t value)
		{
			asm volatile("outb %0, %1" 
						: 
						: "a"(value), "Nd"(port)
						);
		}
		
		void Word(uint16_t port, uint16_t value)
		{
			asm volatile("outw %0, %1" 
						: 
						: "a"(value), "Nd"(port)
						);
		}
		
		void Dword(uint16_t port, uint32_t value)
		{
			asm volatile("outl %0, %1" 
						: 
						: "a"(value), "Nd"(port)
						);
		}
	}	

	namespace OutString
	{
		void Word(uint16_t port, uintptr_t addr, uint32_t cnt)
		{
			asm volatile("cld; rep outsw" 
						: "=D" (addr), "=c" (cnt) 
						: "d" (port), "0" (addr), "1" (cnt) 
						: "memory", "cc");
		}

		void Dword(uint16_t port, uintptr_t addr, uint32_t cnt)
		{
			asm volatile("cld; rep outsl" 
						: "=D" (addr), "=c" (cnt) 
						: "d" (port), "0" (addr), "1" (cnt) 
						: "memory", "cc");
		}
	}
}