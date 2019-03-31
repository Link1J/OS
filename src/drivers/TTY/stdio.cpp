#include "stdio.hpp"
#include "VFS.hpp"
#include "printf.h"
#include "IO.hpp"

stdio::stdio(const char* input, const char* output)
    : Device("/System", "stdio")
{
    inputFile = VFS::OpenFile(input);
    outputFile = VFS::OpenFile(output);
}


uint64_t stdio::Read(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    return VFS::ReadFile(inputFile, buffer, bufferSize);   
}

void stdio::Write(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    VFS::WriteFile(outputFile, buffer, bufferSize);
}

static uint64_t stdioFile = 0;

void stdio::Init()
{
    new stdio("/System/input/keyboard", "/System/tty/screen");
    stdioFile = VFS::OpenFile("/System/stdio");
    if (stdioFile == 0)
        printf("Failed to open stdio\n");
} 

uint64_t stdio::File()
{
    return stdioFile;
}

#define PORT 0x3f8   /* COM1 */

void InitSerial()
{
   IO::Out::Byte(PORT + 1, 0x00);    // Disable all interrupts
   IO::Out::Byte(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   IO::Out::Byte(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   IO::Out::Byte(PORT + 1, 0x00);    //                  (hi byte)
   IO::Out::Byte(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   IO::Out::Byte(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   IO::Out::Byte(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int IsEmpty() {
	return (IO::In::Byte(PORT + 5) & 0x20) != 0;
}

void WriteSerial(char a) {
	while (IsEmpty() == 0);
	IO::Out::Byte(PORT, a);
}

void _putchar(char character)
{
    WriteSerial(character);
    if (stdioFile != 0)
        VFS::WriteFile(stdioFile, &character, 1);
}

