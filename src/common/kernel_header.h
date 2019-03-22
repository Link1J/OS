#ifndef __KERNEL_HEADER_H__
#define __KERNEL_HEADER_H__

#include <stdint.h>
#include <stdbool.h>

struct ModuleDescriptor {
    uint64_t numPages;
    uint8_t* buffer;
};

#ifndef __cplusplus
typedef struct ModuleDescriptor ModuleDescriptor;
#endif

struct PhysicalMapSegment {
    uint64_t base;
    uint64_t numPages;

    struct PhysicalMapSegment* next;
    struct PhysicalMapSegment* prev;
};

#ifndef __cplusplus
typedef struct PhysicalMapSegment PhysicalMapSegment;
#endif

struct KernelHeader {
    ModuleDescriptor	kernelImage;

    uint32_t			screenWidth;
    uint32_t			screenHeight;
    uint32_t			screenBufferPages;
    uint32_t*			screenBuffer;
    bool 				screenColorsInverted;

    PhysicalMapSegment*	physMapStart;
    PhysicalMapSegment*	physMapEnd;

    void*				stack;
    uint64_t			stackPages;

    uint64_t*			pageBuffer;
    uint64_t			pageBufferPages;
    uint64_t			highMemoryBase;
};

#ifndef __cplusplus
typedef struct KernelHeader KernelHeader;
#endif

#endif