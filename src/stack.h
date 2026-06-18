#ifndef STACK_H
#define STACK_H

#include <lib6502/6502.h>

void stack_push_u8(cpu6502 *cpu, uint8_t value);

uint8_t stack_pop_u8(cpu6502 *cpu);

void stack_push_u16(cpu6502 *cpu, uint16_t value);

uint16_t stack_pop_u16(cpu6502 *cpu);

#endif // STACK_H