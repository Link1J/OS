#include <cstdint>

namespace IO
{
	namespace In
	{
		uint8_t  Byte (uint16_t port);
		uint16_t Word (uint16_t port);		
		uint32_t Dword(uint16_t port);
	}
	
	namespace InString
	{
		void Word (uint16_t port, uintptr_t addr, uint32_t cnt);
		void Dword(uint16_t port, uintptr_t addr, uint32_t cnt);
	}

	namespace Out
	{
		void Byte (uint16_t port, uint8_t  value);
		void Word (uint16_t port, uint16_t value);
		void Dword(uint16_t port, uint32_t value);
	}

	namespace OutString
	{
		void Word (uint16_t port, uintptr_t addr, uint32_t cnt);
		void Dword(uint16_t port, uintptr_t addr, uint32_t cnt);
	}
}

