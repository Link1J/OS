#include <stdint.h>

enum class FIS_Type : uint8_t
{
	REG_H2D	    = 0x27,	// Register FIS - host to device
	REG_D2H	    = 0x34,	// Register FIS - device to host
	DMA_ACT 	= 0x39,	// DMA activate FIS - device to host
	DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	DATA		= 0x46,	// Data FIS - bidirectional
	BIST		= 0x58,	// BIST activate FIS - bidirectional
	PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} ;

struct FIS_Register_HostToDevice
{ 
	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:3;		// Reserved
	uint8_t  c:1;		// 1: Command, 0: Control
	uint8_t  command;	// Command register
	uint8_t  featurel;	// Feature register, 7:0
	uint8_t  lba0;		// LBA low register, 7:0
	uint8_t  lba1;		// LBA mid register, 15:8
	uint8_t  lba2;		// LBA high register, 23:16
	uint8_t  device;		// Device register
	uint8_t  lba3;		// LBA register, 31:24
	uint8_t  lba4;		// LBA register, 39:32
	uint8_t  lba5;		// LBA register, 47:40
	uint8_t  featureh;	// Feature register, 15:8
	uint8_t  countl;		// Count register, 7:0
	uint8_t  counth;		// Count register, 15:8
	uint8_t  icc;		// Isochronous command completion
	uint8_t  control;	// Control register
	uint8_t  rsv1[4];	// Reserved
} __attribute__ ((packed));

struct FIS_Register_DeviceToHost
{
	uint8_t  pmport:4;    // Port multiplier
	uint8_t  rsv0:2;      // Reserved
	uint8_t  i:1;         // Interrupt bit
	uint8_t  rsv1:1;      // Reserved
	uint8_t  status;      // Status register
	uint8_t  error;       // Error register
	uint8_t  lba0;        // LBA low register, 7:0
	uint8_t  lba1;        // LBA mid register, 15:8
	uint8_t  lba2;        // LBA high register, 23:16
	uint8_t  device;      // Device register
	uint8_t  lba3;        // LBA register, 31:24
	uint8_t  lba4;        // LBA register, 39:32
	uint8_t  lba5;        // LBA register, 47:40
	uint8_t  rsv2;        // Reserved
	uint8_t  countl;      // Count register, 7:0
	uint8_t  counth;      // Count register, 15:8
	uint8_t  rsv3[2];     // Reserved
	uint8_t  rsv4[4];     // Reserved
} __attribute__ ((packed));

struct FIS_Data
{
	uint8_t  pmport:4;	// Port multiplier
	uint8_t  rsv0:4;	// Reserved
	uint8_t  rsv1[2];	// Reserved
	uint32_t data[1];	// Payload
} __attribute__ ((packed));

struct FIS
{
    FIS_Type type;

    union 
    {
        FIS_Register_HostToDevice   regH2D;
        FIS_Register_DeviceToHost   regD2H;
        FIS_Data                    data;
    };
} __attribute__ ((packed));
