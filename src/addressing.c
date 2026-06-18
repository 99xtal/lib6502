#include <lib6502/6502.h>
#include "opcodes.h"

Operand addr_imm(cpu6502 *cpu) {
  return (Operand) {
    .addr = cpu->PC++,
    .page_crossed = 0
  };
}

Operand addr_abs(cpu6502 *cpu) {
  uint8_t low = cpu->read(cpu->ctx, cpu->PC);
  cpu->PC++;
  uint8_t high = cpu->read(cpu->ctx, cpu->PC);
  cpu->PC++;

  uint16_t addr = ((uint16_t)high << 8) | low;

  return (Operand) {
    .addr = addr,
    .page_crossed = 0
  };
}

Operand addr_abs_x(cpu6502 *cpu) {
  uint8_t low  = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t high = cpu->read(cpu->ctx, cpu->PC++);

  uint16_t base = ((uint16_t)high << 8) | low;
  uint16_t addr = base + cpu->X;

  return (Operand) {
    .addr = addr,
    .page_crossed = (base & 0xFF00) != (addr & 0xFF00)
  };
}

Operand addr_abs_y(cpu6502 *cpu) {
  uint8_t low  = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t high = cpu->read(cpu->ctx, cpu->PC++);

  uint16_t base = ((uint16_t)high << 8) | low;
  uint16_t addr = base + cpu->Y;

  return (Operand) {
    .addr = addr,
    .page_crossed = (base & 0xFF00) != (addr & 0xFF00)
  };
}

Operand addr_indirect_x(cpu6502 *cpu) {
  uint8_t base = cpu->read(cpu->ctx, cpu->PC++);

  uint8_t zero_page_pointer = base + cpu->X;

  uint8_t low = cpu->read(cpu->ctx, zero_page_pointer);
  uint8_t high = cpu->read(cpu->ctx, (uint8_t)(zero_page_pointer + 1));

  return (Operand) {
    .addr = ((uint16_t)high << 8 | low),
    .page_crossed = 0
  };
}

Operand addr_indirect_y(cpu6502 *cpu) {
  uint8_t pointer = cpu->read(cpu->ctx, cpu->PC++);

  uint8_t low = cpu->read(cpu->ctx, pointer);
  uint8_t high = cpu->read(cpu->ctx, (uint8_t)(pointer + 1));

  uint16_t base = ((uint16_t)high << 8) | low;
  uint16_t addr = base + cpu->Y;

  return (Operand) {
      .addr = addr,
      .page_crossed = (base & 0xFF00) != (addr & 0xFF00)
  };
}

Operand addr_zero_page(cpu6502 *cpu) {
  uint8_t addr = cpu->read(cpu->ctx, cpu->PC);
  cpu->PC++;

  return (Operand) {
    .addr = addr,
    .page_crossed = 0
  };
}

Operand addr_zero_page_x(cpu6502 *cpu) {
  uint8_t base = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t addr = base + cpu->X;

  return (Operand) {
    .addr = addr,
    .page_crossed = 0
  };
}

Operand addr_implied(cpu6502 *cpu __attribute__((unused))) {
  return (Operand) { 
    .addr = 0,
    .page_crossed = 0
  };
}