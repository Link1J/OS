#ifndef __MEMORYMAP_H__
#define __MEMORYMAP_H__

#include <stdint.h>
#include "kernel_header.h"

void getMemoryMap();
void printMemoryInfo();
void cleanUpMemMap(uint8_t* memMap, uint64_t size, uint64_t descSize, uint32_t* counter, uint32_t* total);
PhysicalMapSegment* build();

#endif