#ifndef __PCI_HPP__
#define __PCI_HPP__

#include "Device.hpp"

class PCIDevice : public Device
{
    uint8_t bus, device, function;
public:
    PCIDevice(const char* folder, const char* name, uint8_t bus, uint8_t device, uint8_t function);
                
    virtual uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize);
    virtual void Write(uint64_t pos, void* buffer, uint64_t bufferSize);
};

namespace PCI
{
    void Init();

    uint32_t ReadConfigDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
    uint16_t ReadConfigWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
    uint8_t ReadConfigByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
}

#endif