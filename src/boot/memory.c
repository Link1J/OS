#include "memory.h"

void* allocate(uint64_t size, EFI_MEMORY_TYPE type)
{
    void* res = (void*)1; // the pointer is not allowed to be 0 (no idea why)
    BS->AllocatePages(AllocateAnyPages, type, (size + 4095) / 4096, (EFI_PHYSICAL_ADDRESS*)&res);
    return res;
}

void free(void* block, uint64_t size)
{
    BS->FreePages((EFI_PHYSICAL_ADDRESS)block, (size + 4095) / 4096);
}

