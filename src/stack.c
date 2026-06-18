#include <lib6502/6502.h>

void stack_push_u8(cpu6502 *cpu, uint8_t value) {
  cpu->write(cpu->ctx, 0x0100 | cpu->SP, value);
  cpu->SP--;
}

uint8_t stack_pop_u8(cpu6502 *cpu) {
  cpu->SP++;
  return cpu->read(cpu->ctx, 0x0100 | cpu->SP);
}

void stack_push_u16(cpu6502 *cpu, uint16_t value) {
  stack_push_u8(cpu, (value >> 8) & 0xFF);
  stack_push_u8(cpu, value & 0xFF);
}

uint16_t stack_pop_u16(cpu6502 *cpu) {
  uint8_t lo = stack_pop_u8(cpu);
  uint8_t hi = stack_pop_u8(cpu);
  return (uint16_t)(lo | (hi << 8));
}