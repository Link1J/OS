#include "util.h"
#include <stdbool.h>

EFI_LOADED_IMAGE_PROTOCOL* LI;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FS;
EFI_GRAPHICS_OUTPUT_PROTOCOL* G;
EFI_HANDLE IH;

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