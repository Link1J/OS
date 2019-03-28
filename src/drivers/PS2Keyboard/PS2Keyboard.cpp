#include "PS2Keyboard.hpp"
#include "IO.hpp"
#include "TTY/stdio.hpp"
#include "VFS.hpp"

#define STATUS_PORT 0x64
#define DATA_PORT 0x60

static bool shift = false;
static bool caps = false;

static char scancodeSet1Table1[] =
{
    '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0',
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' ', '\0'
};

static char scancodeSet1Table1Shift[] =
{
    '\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' ', '\0'
};

PS2Keyboard::PS2Keyboard() : Device("", "keyboard") 
{
    device = this;
}

uint64_t PS2Keyboard::Read(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    uint64_t size = 0;
    char letter = 0;

    if (device->max > 0)
    {
        uint8_t bufferKeyboard = device->buffer[--device->max];
    
        if (bufferKeyboard == 0x2A || bufferKeyboard == 0x36)
            shift = true;
        if (bufferKeyboard == 0xAA || bufferKeyboard == 0xB6)
            shift = false;
        if (bufferKeyboard == 0x3A)
            caps = !caps;

        if (bufferKeyboard < 0x58)
        {
            if (bufferKeyboard < sizeof(scancodeSet1Table1))
            {
                letter = scancodeSet1Table1[bufferKeyboard];

                if ((letter >= 'a' && letter <= 'z') && caps)
                    letter -= 0x20;

                if (shift)
                    letter = scancodeSet1Table1Shift[bufferKeyboard];
            }
        }        
    }

    ((char*)buffer)[0] = letter;
    return (letter != 0) ? 1 : 0;
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