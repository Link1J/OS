#include "Device.hpp"

namespace PCI
{
    class PCIDevice : public Device
    {
        uint8_t bus, device, function;
    public:
        PCIDevice(const char* folder, const char* name, uint8_t bus, uint8_t device, uint8_t function);
                
        uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize);
        void Write(uint64_t pos, void* buffer, uint64_t bufferSize);
    };

    void Init();
}