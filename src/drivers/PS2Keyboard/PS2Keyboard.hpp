#include "Device.hpp"

class PS2Keyboard : public Device
{
    static PS2Keyboard * device;
    char buffer[100];
    int max = 0;
public:
    PS2Keyboard();
    uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize);
    void Write(uint64_t pos, void* buffer, uint64_t bufferSize);
    static void ReadPort();
};