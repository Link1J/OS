#ifndef __PAGING_H__
#define __PAGING_H__

#include "kernel_header.h"

void initPaging(KernelHeader* header);
void* convertPtr(void* pointer);

#endif
