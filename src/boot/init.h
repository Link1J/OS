#ifndef __INIT_H__
#define __INIT_H__

#include "kernel_header.h"

void initBootloader();
void setVideoMode();
void initKernelHeader(KernelHeader** header);

#endif