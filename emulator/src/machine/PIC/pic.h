#ifndef NA_16_PIC_H
#define NA_16_PIC_H

#include "../machine.h"

#define EXCEPTION_COUNT 0x20

typedef enum {
    DVZ,    // DiVide by Zero
    UO,     // Unknown Opcode
    PV,     // Privilege Violation
    PF,     // Page Fault
    GP,     // General Protection
} Exceptions;

void raise_exception(Machine *machine, Exceptions code, u64 address);
void pic_raise(InterruptController *pic, u8 code);

void enter_exception(Machine *machine);
void enter_irq(Machine *machine);

#endif //NA_16_PIC_H
