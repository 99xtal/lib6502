#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <lib6502/6502.h>
#include "opcodes.h"

/** Status Flag Changes */
int clc(cpu6502 *cpu, Operand op);  // Clear carry flag
int cld(cpu6502 *cpu, Operand op);  // Clear decimal mode flag
int cli(cpu6502 *cpu, Operand op);  // Clear interrupt disable flag
int clv(cpu6502 *cpu, Operand op);  // Clear overflow flag
int sec(cpu6502 *cpu, Operand op);  // Set carry flag
int sed(cpu6502 *cpu, Operand op);  // Set decimal mode flag
int sei(cpu6502 *cpu, Operand op);  // Set interrupt disable flag


int lda(cpu6502 *cpu, Operand op);

int nop(cpu6502 *cpu, Operand op);

#endif // INSTRUCTIONS_H