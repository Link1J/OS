#include "stdio.hpp"
#include "VFS.hpp"
#include "printf.h"

stdio::stdio(const char* input, const char* output)
    : Device("", "stdio")
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

uint64_t stdioFile = 0;

void stdio::Init()
{
    new stdio("/Devices/keyboard", "/Devices/tty/screen");
    stdioFile = VFS::OpenFile("/Devices/stdio");
}

void _putchar(char character)
{
    if (stdioFile != 0)
        VFS::WriteFile(stdioFile, &character, 1);
}

