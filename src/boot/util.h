#ifndef __UTIL_H__
#define __UTIL_H__

#include <efi.h>
#include <efilib.h>

#include "printf.h"

extern EFI_LOADED_IMAGE_PROTOCOL* LI;
extern EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FS;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL* G;
extern EFI_HANDLE IH;
extern UINTN mapKey;

void  waitForKey();
void  exit(EFI_STATUS code);
void  closeBootServices();
void* allocate(uint64_t size, EFI_MEMORY_TYPE type);
void  release(void* block, uint64_t size);

#endif