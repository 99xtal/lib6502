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

Operand addr_zero_page_y(cpu6502 *cpu) {
  uint8_t base = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t addr = base + cpu->Y;

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

Operand addr_indirect(cpu6502 *cpu) {
  uint8_t ptr_low  = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t ptr_high = cpu->read(cpu->ctx, cpu->PC++);

  uint16_t ptr = ((uint16_t)ptr_high << 8) | ptr_low;

  uint8_t target_low = cpu->read(cpu->ctx, ptr);

  /**
   * Original 6502 JMP ($xxxx) bug.
   *
   * The CPU incorrectly wraps the high-byte fetch within the same page.
   *
   * Example:
   *   JMP ($30FF)
   *   low byte  = memory[$30FF]
   *   high byte = memory[$3000]  // should be $3100
   */
  uint16_t target_high_addr =
    (ptr & 0xFF00) | ((ptr + 1) & 0x00FF);

  uint8_t target_high = cpu->read(cpu->ctx, target_high_addr);

  return (Operand) {
    .addr = ((uint16_t)target_high << 8) | target_low,
    .page_crossed = 0
  };
}