SECTIONS
{
    /* Set the location counter to 0 */
    . = 0x0;
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
}
