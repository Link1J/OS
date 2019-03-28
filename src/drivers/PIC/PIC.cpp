#include "PIC.hpp"
#include "IO.hpp"
#include "PS2Keyboard/PS2Keyboard.hpp"
#include "printf.h"

#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

#define PIC_EOI         0x20        /* End-of-interrupt command code */

#define ICW1_ICW4	    0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	    0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	    0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	    0x10		/* Initialization - required! */
 
#define ICW4_8086	    0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	    0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	    0x10		/* Special fully nested (not) */

void Remap(int offset1, int offset2)
{
	unsigned char a1, a2;
 
	a1 = IO::In::Byte(PIC1_DATA);                        // save masks
	a2 = IO::In::Byte(PIC2_DATA);
 
	IO::Out::Byte(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	//io_wait();
	IO::Out::Byte(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	//io_wait();
	IO::Out::Byte(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	//io_wait();
	IO::Out::Byte(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	//io_wait();
	IO::Out::Byte(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	//io_wait();
	IO::Out::Byte(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	//io_wait();
 
	IO::Out::Byte(PIC1_DATA, ICW4_8086);
	//io_wait();
	IO::Out::Byte(PIC2_DATA, ICW4_8086);
	//io_wait();
 
	IO::Out::Byte(PIC1_DATA, a1);   // restore saved masks.
	IO::Out::Byte(PIC2_DATA, a2);
}

void SendEOI(unsigned char irq)
{
	if(irq >= 8)
		IO::Out::Byte(PIC2_COMMAND,PIC_EOI);
 
	IO::Out::Byte(PIC1_COMMAND,PIC_EOI);
}

void IRQSetMask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = IO::In::Byte(port) | (1 << IRQline);
    IO::Out::Byte(port, value);        
}
 
void IRQClearMask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = IO::In::Byte(port) & ~(1 << IRQline);
    IO::Out::Byte(port, value);        
}

void PIC::EnableTimerIRQ()
{
    IRQClearMask(0);
}

void PIC::EnableKeyboardIRQ()
{
    IRQClearMask(1);
    IDT::SetInterruptHandler(33, &PIC::KeyboardInterrupt);
}

void PIC::KeyboardInterrupt(IDT::Registers* regs)
{
    PS2Keyboard::ReadPort();
    SendEOI(1);
}

void PIC::Init()
{
    printf("Setting up PIC\n");

    Remap(0x20, 0x28);

    printf("PIC setup completed\n");
}