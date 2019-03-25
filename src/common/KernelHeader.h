#pragma once

#include <stdint.h>

struct ModuleDescriptor {
    uint64_t numPages;
    uint8_t* buffer;
};

struct PhysicalMapSegment {
    uint64_t base;
    uint64_t numPages;

    PhysicalMapSegment* next;
    PhysicalMapSegment* prev;
};

struct KernelHeader {
    ModuleDescriptor kernelImage;
    //ModuleDescriptor ramdiskImage;

    uint32_t screenWidth;
    uint32_t screenHeight;
    uint32_t screenScanlineWidth;
    uint32_t screenBufferPages;
    uint32_t* screenBuffer;
    bool screenColorsInverted;

    PhysicalMapSegment* physMapStart;
    PhysicalMapSegment* physMapEnd;

    void* stack;
    uint64_t stackPages;

    uint64_t* pageBuffer;
    uint64_t pageBufferPages;
    uint64_t highMemoryBase;
	
	void* RSDPStructure;
};