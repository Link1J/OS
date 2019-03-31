#include "PCI.hpp"
#include "IO.hpp"
#include "VFS.hpp"
#include "printf.h"
#include <string.h>

#define __PCI_CPP__
#include "DeviceList.hpp"

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

    void CheckBus(uint8_t bus, char* folder);

    void CheckFunction(uint8_t bus, uint8_t device, uint8_t function, char* folder) {
        uint8_t baseClass;
        uint8_t subClass;
        uint8_t secondaryBus;
        uint8_t progIF;

        int nameSize = strlen(folder);
        char* name = new char[nameSize + 2];
        snprintf(name, 2, "%d", function);

        baseClass = ReadConfigByte(bus, device, function, 11);
        subClass = ReadConfigByte(bus, device, function, 10);        
        progIF = PCI::ReadConfigByte(bus, device, function,  9);

        if((baseClass == 0x06) && (subClass == 0x04)) 
        {
            VFS::CreateFolder(folder, name);
            snprintf(name, nameSize + 2, "%s/%d", folder, function);

            secondaryBus = ReadConfigByte(bus, device, function, 0x16);
            CheckBus(secondaryBus, name);
        }
        else
        {
            PCIDevice* pciDevice = new PCIDevice(folder, name, bus, device, function);
            snprintf(name, nameSize + 2, "%s/%d", folder, function);

            for (int a = 0; createDeviceList[a].function != 0x0; a++)
            {
                if (createDeviceList[a].classCode == baseClass &&
                    createDeviceList[a].subclass  == subClass  &&
                    createDeviceList[a].progIF    == progIF     )
                {
                    createDeviceList[a].function(bus, device, function, name);
                }
            } 
        }
        delete[] name;
    }
    
    void CheckDevice(uint8_t bus, uint8_t device, const char* folder) 
    {
        uint8_t function = 0;
 
        uint16_t vendorID = GetVendorID(bus, device, function);
        if(vendorID == 0xFFFF) return;        // Device doesn't exist

        int nameSize = strlen(folder);
        char* name = new char[nameSize + 4];
        snprintf(name, 3, "%d", device);
        VFS::CreateFolder(folder, name);
        snprintf(name, nameSize + 3, "%s/%d", folder, device);
        CheckFunction(bus, device, function, name);
        auto headerType = GetHeaderType(bus, device, function);
        if( (headerType & 0x80) != 0) {
         for(function = 1; function < 8; function++) {
             if(GetVendorID(bus, device, function) != 0xFFFF) {
                 CheckFunction(bus, device, function, name);
             }
         }
     }

        delete[] name;
    }
 
    void CheckBus(uint8_t bus, char* folder) 
    {
        int nameSize = strlen(folder);
        char* name = new char[nameSize + 6];
        snprintf(name, 4, "%d", bus);
        VFS::CreateFolder(folder, name);
        snprintf(name, nameSize + 6, "%s/%d", folder, bus);

        uint8_t device;
 
        for(device = 0; device < 32; device++) {
            CheckDevice(bus, device, name);
        }

        delete[] name;
    }

    void CheckAllBuses() 
    {
        uint8_t function;
        uint8_t bus;
 
        auto headerType = GetHeaderType(0, 0, 0);
        if( (headerType & 0x80) == 0) 
        {
            /* Single PCI host controller */
            CheckBus(0, "/System/PCI");
        }
        else 
        {
            /* Multiple PCI host controllers */
            for(function = 0; function < 8; function++) 
            {
                if(GetVendorID(0, 0, function) != 0xFFFF) break;
                bus = function;
                CheckBus(bus, "/System/PCI");
            }
        }
    }

    void Init()
    {
        if (!VFS::CreateFolder("/System", "PCI"))
            printf("Failed to create /System/PCI folder!");

        CheckAllBuses();
    }
}

PCIDevice::PCIDevice(const char* folder, const char* name, uint8_t bus, uint8_t device, uint8_t function)
        : Device(folder, name), bus(bus), device(device), function(function) {}
                
uint64_t PCIDevice::Read(uint64_t pos, void* buffer, uint64_t bufferSize)
{
    char tempBuffer[200];

    uint16_t vendorID  = PCI::GetVendorID   (bus, device, function    );
    uint16_t deviceID  = PCI::ReadConfigWord(bus, device, function,  2);
    uint8_t  classCode = PCI::ReadConfigByte(bus, device, function, 11);
    uint8_t  subclass  = PCI::ReadConfigByte(bus, device, function, 10);
    uint8_t  progIF    = PCI::ReadConfigByte(bus, device, function,  9);

    int size = snprintf(tempBuffer, 200, 
    "Bus: %d, Device: %d, Function: %d\n\tVendor    : 0x%04X\n\tDevice    : 0x%04X\n\tClass Code: 0x%02X\n\tSubclass  : 0x%02X\n\tProg IF   : 0x%02X\n", 
    bus, device, function, vendorID, deviceID, classCode, subclass, progIF);

    memcpy(buffer, tempBuffer + pos, (size - pos < bufferSize) ? size - pos + 1 : bufferSize);
        
    return strlen((char*)buffer);
}

void PCIDevice::Write(uint64_t pos, void* buffer, uint64_t bufferSize)
{

}