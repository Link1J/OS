#include "graphics.h"
#include "util.h"

void setVideoMode()
{
	printf("Switching video mode...\n");

    // find best resolution the Graphics Protocol supports
    UINT32 resBestX = 0;
    UINT32 resBestY = 0;
    UINT32 resBestScanlineWidth = 0;
    UINT32 resBestMode = 0;
    for(UINT32 m = 0; m < G->Mode->MaxMode; m++) {
        UINTN infoSize;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
		G->QueryMode(G, m, &infoSize, &info);

        // retrict to 1920x1080, as otherwise qemu tends to create huge windows that do not fit onto the screen
        if(info->VerticalResolution > resBestY && info->HorizontalResolution > resBestX && info->HorizontalResolution <= 1280 && info->VerticalResolution <= 720 && (info->PixelFormat == PixelBlueGreenRedReserved8BitPerColor || info->PixelFormat == PixelRedGreenBlueReserved8BitPerColor)) {
            resBestX = info->HorizontalResolution;
            resBestY = info->VerticalResolution;
            resBestScanlineWidth = info->PixelsPerScanLine;
            resBestMode = m;
        }
    }

    G->SetMode(G, resBestMode);
}