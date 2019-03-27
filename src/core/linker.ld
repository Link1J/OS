OUTPUT_FORMAT(elf64-x86-64)
ENTRY(KernelMain)

PAGE_SIZE  = 0x1000;
KERNEL_VMA = 0xFFFFFFFF80000000;
UPPER_MEM  = 0x100000;

SECTIONS
{
  . = UPPER_MEM;
  _start = . + KERNEL_VMA;

  .init :
  {
    *(.initl)
  }

  . += KERNEL_VMA;

  .text ALIGN(PAGE_SIZE) : AT(ADDR(.text) - KERNEL_VMA)
  {
    *(.inith)
    *(.text*)
    *(.gnu.linkonce.t*)
  }

  .data ALIGN(PAGE_SIZE) : AT(ADDR(.data) - KERNEL_VMA)
  {
    *(.data)
    *(.gnu.linkonce.d*)
  }

  .rodata ALIGN(PAGE_SIZE) : AT(ADDR(.rodata) - KERNEL_VMA)
  {
     start_ctors = .;
     *(SORT(.ctors*))  /* Note the "SORT" */
     end_ctors = .;

     start_dtors = .;
     *(SORT(.dtors*))
     end_dtors = .;

     *(.rodata*)
     *(.gnu.linkonce.r*)
  }

  .eh_frame_hdr ALIGN(PAGE_SIZE) : AT(ADDR(.eh_frame_hdr) - KERNEL_VMA)
  {
    *(.eh_frame_hdr*)
  }

  .eh_frame ALIGN(PAGE_SIZE) : AT(ADDR(.eh_frame) - KERNEL_VMA)
  {
    start_eh_frame = .;
    *(.eh_frame*)
  }

  .gcc_except_table ALIGN(PAGE_SIZE) : AT(ADDR(.gcc_except_table) - KERNEL_VMA)
  {
    *(.gcc_except_table*)
  }

  .bss ALIGN(PAGE_SIZE) : AT(ADDR(.bss) - KERNEL_VMA)
  {
    *(COMMON)
    *(.bss)
    *(.gnu.linkonce.b*)
  }

  _end = .;

  /DISCARD/ :
  {
    *(.comment)
  }
}