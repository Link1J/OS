#include "PCI.hpp"
#include "IO.hpp"
#include "VFS.hpp"
#include "printf.h"

namespace PCI
{
    uint32_t ReadConfigDword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) 
    {
        uint32_t address;
        uint32_t lbus  = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;
        uint32_t tmp = 0;
    
        address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    
        IO::Out::Dword(0xCF8, address);
        return IO::In::Dword(0xCFC);
    }

    uint16_t ReadConfigWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) 
    {
        return (uint16_t)((ReadConfigDword(bus, slot, func, offset) >> ((offset & 2) * 8)) & 0xffff);
    }

    uint8_t ReadConfigByte(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) 
    {
        return (uint8_t)((ReadConfigDword(bus, slot, func, offset) >> ((offset & 3) * 8)) & 0xff);
    }

    uint16_t GetVendorID(uint8_t bus, uint8_t slot, uint8_t func)
    {
        return ReadConfigWord(bus, slot, func, 0);
    }

    uint8_t GetHeaderType(uint8_t bus, uint8_t slot, uint8_t func)
    {
        return ReadConfigByte(bus, slot, func, 0xE);
    }

    PCIDevice::PCIDevice(const char* folder, const char* name, uint8_t bus, uint8_t device)
        : Device(folder, name), bus(bus), device(device) {}
                
    uint64_t PCIDevice::Read(uint64_t pos, void* buffer, uint64_t bufferSize)
    {
        uint16_t vendorID  = GetVendorID   (bus, device, 0    );
        uint16_t deviceID  = ReadConfigWord(bus, device, 0,  2);
        uint8_t  classCode = ReadConfigByte(bus, device, 0, 11);
        uint8_t  subclass  = ReadConfigByte(bus, device, 0, 10);
        uint8_t  progIF    = ReadConfigByte(bus, device, 0,  9);

        uint64_t size = snprintf((char*)buffer, bufferSize, 
        "Bus: %d, Device: \n\tVendor    : 0x%04X\n\tDevice    : 0x%04X\n\tClass Code: 0x%02X\n\tSubclass  : 0x%02X\n\tProg IF   : 0x%02X\n", 
        bus, device, vendorID, deviceID, classCode, subclass, progIF);

        return size;
    }

    void PCIDevice::Write(uint64_t pos, void* buffer, uint64_t bufferSize)
    {

    }
    
    void CheckDevice(uint8_t bus, uint8_t device, const char* folder) 
    {
        uint8_t function = 0;
 
        uint16_t vendorID = GetVendorID(bus, device, function);
        if(vendorID == 0xFFFF) return;        // Device doesn't exist
        
        char name[3];
        snprintf(name, 3, "%d", device);
        
        new PCIDevice(folder, name, bus, device);
    }
 
    void CheckBus(uint8_t bus) 
    {
        char name[16];
        snprintf(name, 4, "%d", bus);
        VFS::CreateFolder("/System/PCI", name);
        snprintf(name, 16, "/System/PCI/%d", bus);

        uint8_t device;
 
        for(device = 0; device < 32; device++) {
            CheckDevice(bus, device, name);
        }
    }

    void CheckAllBuses() 
    {
        uint8_t function;
        uint8_t bus;
 
        auto headerType = GetHeaderType(0, 0, 0);
        if( (headerType & 0x80) == 0) 
        {
            /* Single PCI host controller */
            CheckBus(0);
        }
        else 
        {
            /* Multiple PCI host controllers */
            for(function = 0; function < 8; function++) 
            {
                if(GetVendorID(0, 0, function) != 0xFFFF) break;
                bus = function;
                CheckBus(bus);
            }
        }
    }

    void Init()
    {
        if (!VFS::CreateFolder("/System", "PCI"))
            printf("Failed to create /System/PCI folder!");

        CheckAllBuses();
    }

    /*struct BISTRegister
    {
        uint8_t capable : 1;
        uint8_t start : 1;
        uint8_t reserved : 2;
        uint8_t completionCode : 4;
    } __attribute__ ((packed));

    struct HeaderTypeRegister
    {
        uint8_t multipleFunctions : 1;
        uint8_t type : 7;
    } __attribute__ ((packed));

    struct CommandRegister
    {
        uint16_t reserved0 : 4;
        uint16_t interruptDissable : 1;
        //uint16_t interruptDissable : 1;
    } __attribute__ ((packed));

    struct PCI_Device
    {
        uint16_t vendorID;
        uint16_t deviceID;
        uint16_t command;
        uint16_t status;
        uint8_t  revisionID;
        uint8_t  progIF;
        uint8_t  subclass;
        uint8_t  classCode;
        uint8_t  cacheLineSize;
        uint8_t  latencyTimer;
        HeaderTypeRegister headerType;
        BISTRegister bist;
        union
        {
            struct 
            { 
                uint32_t baseAddress0;
                uint32_t baseAddress1;
                uint32_t baseAddress2;
                uint32_t baseAddress3;
                uint32_t baseAddress4;
                uint32_t baseAddress5;
                uint32_t cardbusCISPointer;
                uint16_t subsystemVendorID;
                uint16_t subsystemID;
                uint32_t expansionRomBaseAddress;
                uint8_t  capabilitiesPointer;
                uint16_t reserved0;
                uint8_t  reserved1;
                uint32_t reserved2;
                uint8_t  interruptLine;
                uint8_t  interruptPin;
                uint8_t  minGrant;
                uint8_t  maxLatency;
            } x00 __attribute__ ((packed));
            struct 
            { 
                uint32_t baseAddress0;
                uint32_t baseAddress1;
                uint8_t  primaryBusNumber;
                uint16_t secondaryBusNumber;
                uint8_t  secondaryLatencyTimer;
                uint8_t  ioBase;
                uint8_t  ioLimit;
                uint16_t secondaryStatus;
                uint16_t memoryBase;
                uint16_t memoryLimit;
                uint16_t prefetchableBase;
                uint16_t prefetchableLimit;
                uint32_t prefetchableBaseUpper32;
                uint32_t prefetchableLimitUpper32;
                uint16_t ioBaseUpper16;
                uint16_t ioLimitUpper16;
                uint8_t  capabilitiesPointer;
                uint16_t reserved0;
                uint8_t  reserved1;
                uint32_t expansionRomBaseAddress;
                uint8_t  interruptLine;
                uint8_t  interruptPin;
                uint16_t bridgeControl;
            } x01 __attribute__ ((packed));
            struct 
            { 
                uint32_t baseAddress;
                uint8_t  offsetOfCapabilitiesList;
                uint8_t  reserved;
                uint16_t secondaryStatus;
                uint32_t memoryAddress0;
                uint32_t memoryLimit0;
                uint32_t memoryAddress1;
                uint32_t memoryLimit1;
                uint32_t ioAddress0;
                uint32_t ioLimit0;
                uint32_t ioAddress1;
                uint32_t ioLimit1;
                uint8_t  interruptLine;
                uint8_t  interruptPin;
                uint16_t bridgeControl;
                uint16_t subsystemDeviceID;
                uint16_t subsystemVendorID;
                uint32_t PCCardModeBaseAddress;
            } x02 __attribute__ ((packed));
        }; 
    } __attribute__ ((packed));*/
}
