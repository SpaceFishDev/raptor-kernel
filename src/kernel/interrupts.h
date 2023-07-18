// #ifndef INTERRUPTS_H
// #define INTERRUPTS_H

// #include "../std/stdio.h"
// #include "../std/stdint.h"
// #include "../std/stddef.h"
// #include "../std/memdefs.h"

// void interrupt (*interrupt_table[256])();

// void install_interrupt_handler(int interrupt_number, void interrupt (*handler)())
// {
//     interrupt_table[interrupt_number] = handler;
// }

// void remove_interrupt_handler(int interrupt_number)
// {
//     interrupt_table[interrupt_number] = NULL;
// }

// void interrupt isr()
// {
//     unsigned char interrupt_number;
//     _asm {
//         mov interrupt_number, ah  ; assuming the interrupt number is in AH register
//     }

//     if (interrupt_table[interrupt_number] != NULL)
//     {
//         interrupt_table[interrupt_number]();
//     }
// }

// void install_isr(int interrupt_number)
// {
//     unsigned int isr_address = FP_OFF(isr);
//     unsigned int isr_segment = FP_SEG(isr);
//     unsigned int interrupt_vector = interrupt_number * 4;

//     _asm {
//         push es
//         mov ax, isr_segment
//         mov es, ax
//         mov word ptr es:[interrupt_vector], offset isr_address
//         mov word ptr es:[interrupt_vector + 2], cs
//         pop es
//     }
// }

// #endif
