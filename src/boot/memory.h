#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "util.h"

void* allocate(uint64_t size, EFI_MEMORY_TYPE type);
void free(void* block, uint64_t size);

#endif