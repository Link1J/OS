#ifndef __DEVICELIST_HPP__
#define __DEVICELIST_HPP__

#include <stdint.h>
typedef void (*CreateDevice)(uint8_t bus, uint8_t device, uint8_t function, char* pciDevice);

#ifdef __PCI_CPP__

#include "ATA/ATA.hpp"

struct CreateDeviceItem
{   
    uint8_t  classCode;
    uint8_t  subclass;
    uint8_t  progIF;
    CreateDevice function;
};

CreateDeviceItem createDeviceList[] =
{
    { 0x1, 0x1, 0x80, CreateATADevice },

    { 0x0, 0x0, 0x0, 0x0 },
};

#endif

#endif

