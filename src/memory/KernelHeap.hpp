#include <stdint.h>

namespace KernelHeap 
{
    void* Allocate(uint64_t size);
    void  Free    (void* block  );
}