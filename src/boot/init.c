#include "init.h"
#include "util.h"
#include "paging.h"

UINT32 resBestX = 0;
UINT32 resBestY = 0;
UINT32 resBestMode = 0;

void initBootloader()
{	
	printf("Initializing bootloader\n");
	
	EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_STATUS err = BS->HandleProtocol(IH, &guid, (void**)&LI);
    if(err != EFI_SUCCESS) {
        printf("Failed to init loaded image protocol\nPress any key to exit...\n");
		waitForKey();
        exit(EFI_LOAD_ERROR);
    }
	
	// Get Filesystem access of the device the bootloader is located on
    guid = (EFI_GUID)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    err = BS->HandleProtocol(LI->DeviceHandle, &guid, (void**)&FS);
    if(err != EFI_SUCCESS) {
        printf("Failed to init filesystem protocol\nPress any key to exit...\n");
        waitForKey();
        exit(EFI_LOAD_ERROR);
    }

    // Get Graphics Protocol of the screen
    guid = (EFI_GUID)EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    err = BS->LocateProtocol(&guid, NULL, (void**)&G);
    if(err != EFI_SUCCESS) {
        printf("Failed to init grpahics protocol\nPress any key to exit...\n");
        waitForKey();
        exit(EFI_LOAD_ERROR);
    }
}

void setVideoMode()
{
	printf("Switching video mode\n");

    // find best resolution the Graphics Protocol supports
    for(UINT32 m = 0; m < G->Mode->MaxMode; m++) {
        UINTN infoSize;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
		G->QueryMode(G, m, &infoSize, &info);

        // retrict to 1920x1080, as otherwise qemu tends to create huge windows that do not fit onto the screen
        if(info->VerticalResolution > resBestY && info->HorizontalResolution > resBestX 
		&& info->HorizontalResolution <= 1280 && info->VerticalResolution <= 720 
		&& (info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor || 
		info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)) {
            resBestX = info->HorizontalResolution;
            resBestY = info->VerticalResolution;
            resBestMode = m;
        }
    }

    G->SetMode(G, resBestMode);
}

void fillKernelHeader(KernelHeader* header)
{
	printf("Filling KernelHeader\n");
	header->screenWidth = resBestX;
	header->screenHeight = resBestY;
	header->screenBuffer = (uint32_t*)convertPtr((void*)G->Mode->FrameBufferBase);
	header->screenBufferPages = (G->Mode->FrameBufferSize + 4095) / 4096;
	header->screenColorsInverted = G->Mode->Info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor;
	
	void* newStack = convertPtr(allocate(16 * 4096, (EFI_MEMORY_TYPE)0x80000001));
    header->stack = newStack;
    header->stackPages = 16;
}

void initKernelHeader(KernelHeader** header)
{
	printf("Setting up KernelHeader\n");
	*header = (KernelHeader*)allocate(sizeof(KernelHeader), (EFI_MEMORY_TYPE)0x80000001);
    // Create paging structures to map the topmost 512GB to physical memory
    initPaging(*header);
    // Convert the header pointer to a high-memory pointer
    *header = (KernelHeader*)convertPtr(*header);
	fillKernelHeader(*header);
}