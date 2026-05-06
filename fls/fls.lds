OUTPUT_FORMAT("elf32-c33", "elf32-c33", "elf32-c33")
OUTPUT_ARCH(c33)

__dp            = 0x00000000;
__START_stack   = 0x00004000;

SECTIONS
{
    .bss 0x00000030 :
    {
        __START_bss = . ;
        *(.bss)
        *(.comm)
        __END_bss = . ;
    }

    .data :
    {
        __START_data = . ;
        *(.data)
        __END_data = . ;
    }

    .text 0x00000100 :
    {
        __START_text = . ;
        *(.text)
        __END_text = . ;
    }

    .rodata :
    {
        __START_rodata = . ;
        *(.rodata)
        __END_rodata = . ;
    }
}