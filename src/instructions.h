#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <lib6502/6502.h>
#include "opcodes.h"

/** Load/Store Operations */
int lda(cpu6502 *cpu, Operand op);  // Load accumulator
int ldx(cpu6502 *cpu, Operand op);  // Load X register
int ldy(cpu6502 *cpu, Operand op);  // Load Y register
int sta(cpu6502 *cpu, Operand op);  // Store accumulator
int stx(cpu6502 *cpu, Operand op);  // Store X register
int sty(cpu6502 *cpu, Operand op);  // Store Y register

/** Register transfers */
int tax(cpu6502 *cpu, Operand op);  // Transfer accumulator to X
int tay(cpu6502 *cpu, Operand op);  // Transfer accumulator to Y
int txa(cpu6502 *cpu, Operand op);  // Transfer X to accumulator
int tya(cpu6502 *cpu, Operand op);  // Transfer Y to accumulator

/** Stack Operations */
int tsx(cpu6502 *cpu, Operand op);  // Transfer stack pointer to X
int txs(cpu6502 *cpu, Operand op);  // Transfer X to stack pointer
int pha(cpu6502 *cpu, Operand op);  // Push accumulator on stack
int php(cpu6502 *cpu, Operand op);  // Push processor status on stack
int pla(cpu6502 *cpu, Operand op);  // Pull accumulator from stack
int plp(cpu6502 *cpu, Operand op);  // Pull processor status from stack

/** Logical */
int and(cpu6502 *cpu, Operand op);  // Logical AND
int eor(cpu6502 *cpu, Operand op);  // Exclusive OR
int ora(cpu6502 *cpu, Operand op);  // Logical Inclusive OR
int bit(cpu6502 *cpu, Operand op);  // Bit Test

/** Status Flag Changes */
int clc(cpu6502 *cpu, Operand op);  // Clear carry flag
int cld(cpu6502 *cpu, Operand op);  // Clear decimal mode flag
int cli(cpu6502 *cpu, Operand op);  // Clear interrupt disable flag
int clv(cpu6502 *cpu, Operand op);  // Clear overflow flag
int sec(cpu6502 *cpu, Operand op);  // Set carry flag
int sed(cpu6502 *cpu, Operand op);  // Set decimal mode flag
int sei(cpu6502 *cpu, Operand op);  // Set interrupt disable flag

/** System Functions */
int brk(cpu6502 *cpu, Operand op);  // Force an interrupt
int nop(cpu6502 *cpu, Operand op);  // No operation
int rti(cpu6502 *cpu, Operand op);  // Return from interrupt


#endif // INSTRUCTIONS_H