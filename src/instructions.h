#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <lib6502/6502.h>
#include "opcodes.h"

int lda(cpu6502 *cpu, Operand op);

int nop(cpu6502 *cpu, Operand op);

#endif // INSTRUCTIONS_H