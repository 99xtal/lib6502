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

/** Arithmetic */
int adc(cpu6502 *cpu, Operand op);  // Add with Carry
int sbc(cpu6502 *cpu, Operand op);  // Subtract with Carry
int cmp(cpu6502 *cpu, Operand op);  // Compare accumulator
int cpx(cpu6502 *cpu, Operand op);  // Compare X register
int cpy(cpu6502 *cpu, Operand op);  // Compare Y register

/** Increments & Decrements */
int inc(cpu6502 *cpu, Operand op);  // Increment a memory location
int inx(cpu6502 *cpu, Operand op);  // Increment the X register
int iny(cpu6502 *cpu, Operand op);  // Increment the Y register
int dec(cpu6502 *cpu, Operand op);  // Decrement a memory location
int dex(cpu6502 *cpu, Operand op);  // Decrement the X register
int dey(cpu6502 *cpu, Operand op);  // Decrement the Y register

/** Shifts */
int asl(cpu6502 *cpu, Operand op);  // Arithmetic Shift Left
int lsr(cpu6502 *cpu, Operand op);  // Logical Shift Right
int rol(cpu6502 *cpu, Operand op);  // Rotate Left
int ror(cpu6502 *cpu, Operand op);  // Rotate Right

/** Jumps & Calls */
int jmp(cpu6502 *cpu, Operand op);  // Jump to another location
int jsr(cpu6502 *cpu, Operand op);  // Jump to a subroutine
int rts(cpu6502 *cpu, Operand op);  // Return from subroutine

/** Branches */
int bcc(cpu6502 *cpu, Operand op);  // Branch if carry flag clear
int bcs(cpu6502 *cpu, Operand op);  // Branch if carry flag set
int beq(cpu6502 *cpu, Operand op);  // Branch if zero flag set
int bmi(cpu6502 *cpu, Operand op);  // Branch if negative flag set
int bne(cpu6502 *cpu, Operand op);  // Branch if zero flag clear
int bpl(cpu6502 *cpu, Operand op);  // Branch if negative flag clear
int bvc(cpu6502 *cpu, Operand op);  // Branch if overflow flag clear
int bvs(cpu6502 *cpu, Operand op);  // Branch if overflow flag set

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