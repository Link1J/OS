#include "KernelHeap.hpp"
#include "MemoryList.hpp"
#include "MemoryManager.hpp"

namespace KernelHeap {

    constexpr uint64_t heapBase = ((uint64_t)510 << 39) | 0xFFFF000000000000;

    static MemoryList freeMemory;
    static uint64_t heapPos = HeapBase;

    static void* ReserveNew(uint64_t size) 
    {
        size = (size + 4095) / 4096;
        void* g = MemoryManager::AllocatePages(size);
        for(int i = 0; i < size; i++)
            MemoryManager::MapKernelPage((char*)g + 4096 * i, (char*)g_HeapPos + 4096 * i);
        g_FreeList.MarkFree((void*)g_HeapPos, size * 4096);
        void* ret = (void*)g_HeapPos;
        g_HeapPos += size * 4096;
        return ret;
    }

    void* Allocate(uint64_t size)
    {
        size = (size + sizeof(uint64_t) + 63) / 64 * 64;

        void* mem = freeMemory.FindFree(size);
        if(mem == nullptr)
            mem = ReserveNew(size);
        freeMemory.MarkUsed(mem, size);

        *(uint64_t*)mem = size;
        return (uint64_t*)mem + 1;
    }
	
    void Free(void* block)
    {
        if(block == nullptr)
            return;

        uint64_t* mem = (uint64_t*)block - 1;
        uint64_t size = *mem;

        freeMemory.MarkFree(mem, size);
    }
}
