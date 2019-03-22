#include "util.h"
#include <stdbool.h>

EFI_LOADED_IMAGE_PROTOCOL* LI;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FS;
EFI_GRAPHICS_OUTPUT_PROTOCOL* G;
EFI_HANDLE IH;
UINTN mapKey;

void _putchar(char character)
{
	if (character == '\n')
		_putchar('\r');
	
	wchar_t buffer[] = { character, 0 };
	ST->ConOut->OutputString(ST->ConOut, buffer);
}

void waitForKey()
{
	// clear any pending key presses
	ST->ConIn->Reset(ST->ConIn, false);
	
	EFI_INPUT_KEY key;
	EFI_STATUS ret;
	
	while((ret = ST->ConIn->ReadKeyStroke(ST->ConIn, &key)) == EFI_NOT_READY); // when no key is pressed, function returns EFI_NOT_READY
}

void exit(EFI_STATUS code)
{
	BS->Exit(IH, code, 0, NULL);
}

void closeBootServices()
{	
	printf("Exiting Boot services and starting kernel\nPress any key to continue\n");
	//waitForKey();
	EFI_STATUS err = BS->ExitBootServices(LI, mapKey);
    if(err != EFI_SUCCESS) {
		printf("Failed to exit boot services\nPress any key to exit\n");
		waitForKey();
        exit(EFI_LOAD_ERROR);
    }
}

void* allocate(uint64_t size, EFI_MEMORY_TYPE type)
{
    void* res = (void*)1; // the pointer is not allowed to be 0 (no idea why)
    BS->AllocatePages(AllocateAnyPages, type, (size + 4095) / 4096, (EFI_PHYSICAL_ADDRESS*)&res);
    return res;
}

void release(void* block, uint64_t size)
{
    BS->FreePages((EFI_PHYSICAL_ADDRESS)block, (size + 4095) / 4096);
}

