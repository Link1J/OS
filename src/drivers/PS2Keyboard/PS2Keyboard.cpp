#include "PS2Keyboard.hpp"
#include "IO.hpp"

#define STATUS_PORT 0x64
#define DATA_PORT 0x60

PS2Keyboard::PS2Keyboard() : Device("", "keyboard") 
{
    device = this;
}

uint64_t PS2Keyboard::Read(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    uint64_t size = 0;
    
    while (device->max > 0 && size < bufferSize)
        ((uint8_t*)buffer)[size++] = device->buffer[--device->max];

    return size;
}

void PS2Keyboard::Write(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    uint64_t size = 0;
    while ((IO::In::Byte(STATUS_PORT) & 2) != 0 && size < bufferSize) {}
    while ((IO::In::Byte(STATUS_PORT) & 2) == 0 && size < bufferSize)
       IO::Out::Byte(DATA_PORT, ((uint8_t*)buffer)[size++]);
}

void PS2Keyboard::ReadPort()
{
    while ((IO::In::Byte(STATUS_PORT) & 1) == 1 && device->max < 100)
        device->buffer[device->max++] = IO::In::Byte(DATA_PORT);
}