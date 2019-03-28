#include "Terminal.hpp"
#include "Font3x5.hpp"
#include "FontPC.hpp"
#include "printf.h"
#include "Screen.hpp"
#include "Vector2.hpp"
#include "IO.hpp"

namespace Font = FontPC;

#define PORT 0x3f8   /* COM1 */

namespace Terminal
{
	
	
	/*void InitSerial()
	{
	   IO::Out::Byte(PORT + 1, 0x00);    // Disable all interrupts
	   IO::Out::Byte(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	   IO::Out::Byte(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	   IO::Out::Byte(PORT + 1, 0x00);    //                  (hi byte)
	   IO::Out::Byte(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
	   IO::Out::Byte(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	   IO::Out::Byte(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
	}
	
	void Init()
	{
		InitSerial();
	}
	
	void SetCursor(int x, int y)
	{
		cursor.x = x;
		cursor.y = y;
	}
	
	int is_transmit_empty() {
		return IO::In::Byte(PORT + 5) & 0x20;
	}
 
	void write_serial(char a) {
		while (is_transmit_empty() == 0);
		IO::Out::Byte(PORT, a);
	}
	
	void Hexdump(const char* data, int size)
	{
		printf("\t  %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX\n",
			0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
				
		
		for (int a = 0; a < size; a += 0x10)
		{
			char buffer[17];
			snprintf(buffer, 17, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c", 
				data[a +  0], data[a +  1], data[a +  2], data[a +  3],
				data[a +  4], data[a +  5], data[a +  6], data[a +  7],
				data[a +  8], data[a +  9], data[a + 10], data[a + 11],
				data[a + 12], data[a + 13], data[a + 14], data[a + 15]
				);
			
			printf("%04llX: ", a);
			
			printf("%02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX %02hhX",
				data[a +  0], data[a +  1], data[a +  2], data[a +  3],
				data[a +  4], data[a +  5], data[a +  6], data[a +  7],
				data[a +  8], data[a +  9], data[a + 10], data[a + 11],
				data[a + 12], data[a + 13], data[a + 14], data[a + 15]
				);
				
			printf("\t\t|");	
			
			for (int b = 0; b < 16; b++)
				Terminal::PrintSymbol(buffer[b]);
				
			printf("|\n");
		}
	}
	
	void SetForegoundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		foreground.RGBA.red		= red	;
		foreground.RGBA.green	= green	;
		foreground.RGBA.blue	= blue	;
	}
	
	void SetBackgoundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		background.RGBA.red		= red	;
		background.RGBA.green	= green	;
		background.RGBA.blue	= blue	;
	}
	
	uint32_t X         () { return cursor   .x; }
	uint32_t Y         () { return cursor   .y; }
	uint32_t CharWidth () { return charCount.x; }
	uint32_t CharHeight() { return charCount.y; }*/
}