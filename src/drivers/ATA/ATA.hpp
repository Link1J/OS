#ifndef __ATA_HPP__
#define __ATA_HPP__

#include "PCI/PCI.hpp"
#include "PCI/DeviceList.hpp"

void CreateATADevice(uint8_t bus, uint8_t device, uint8_t function, char* pciDevice);

struct IDEChannelRegisters {
    uint16_t base       ; // I/O Base.
    uint16_t control    ; // Control Base
    uint16_t busMaster  ; // Bus Master IDE
    uint8_t  nIEN       ; // nIEN (No Interrupt);
    uint8_t  channel    ; // 0 (Primary Channel) or 1 (Secondary Channel).
};

void IDEWrite(IDEChannelRegisters& channel, uint8_t reg, uint8_t data);
uint8_t IDERead(IDEChannelRegisters& channel, uint8_t reg);
void IDEReadBuffer(IDEChannelRegisters& channel, uint8_t reg, uintptr_t buffer, uint32_t quads);
uint8_t IDEPolling(IDEChannelRegisters& channel, uint32_t advanced_check);

class ATADevice : public Device
{
    IDEChannelRegisters* channel;
    
    uint8_t  drive;        // 0 (Master Drive) or 1 (Slave Drive).
    uint16_t type;         // 0: ATA, 1:ATAPI.
    uint16_t signature;    // Drive Signature
    uint16_t capabilities; // Features.
    uint32_t commandSets;  // Command Sets Supported.
    uint32_t size;         // Size in Sectors.
    uint8_t  model[41];    // Model in string.

    //uint8_t ide_buf[2048] = {0};
    static uint8_t ide_irq_invoked;
    uint8_t atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    uint8_t PrintError(uint8_t err);
    uint8_t ATAAccess(uint8_t direction, uint32_t lba, uint8_t numsects, uint16_t selector, uintptr_t edi);
public:
    ATADevice(char* device, char* id, IDEChannelRegisters* channel, int number);
                
    uint64_t Read(uint64_t pos, void* buffer, uint64_t bufferSize);
    void Write(uint64_t pos, void* buffer, uint64_t bufferSize);
};

#endif