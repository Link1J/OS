#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__

#include <stdint.h>

class Device
{
public:
    static Device* GetByID(uint64_t id);
    Device(const char* path, const char* name);
            
    virtual uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize) = 0;
    virtual void Write(uint64_t pos, void* buffer, uint64_t bufferSize) = 0;

    uint64_t GetDeviceID() const { return id; }

private:
    uint64_t id;
};

#endif