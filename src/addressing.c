#include <lib6502/6502.h>

#include "addressing.h"
#include "opcodes.h"

Operand addr_imm(cpu6502 *cpu) {
  return (Operand) {
    .type = OPERAND_MEMORY,
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
    .type = OPERAND_MEMORY,
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
    .type = OPERAND_MEMORY,
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
    .type = OPERAND_MEMORY,
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
    .type = OPERAND_MEMORY,
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
      .type = OPERAND_MEMORY,
      .addr = addr,
      .page_crossed = (base & 0xFF00) != (addr & 0xFF00)
  };
}

Operand addr_zero_page(cpu6502 *cpu) {
  uint8_t addr = cpu->read(cpu->ctx, cpu->PC);
  cpu->PC++;

  return (Operand) {
    .type = OPERAND_MEMORY,
    .addr = addr,
    .page_crossed = 0
  };
}

Operand addr_zero_page_x(cpu6502 *cpu) {
  uint8_t base = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t addr = base + cpu->X;

  return (Operand) {
    .type = OPERAND_MEMORY,
    .addr = addr,
    .page_crossed = 0
  };
}

Operand addr_zero_page_y(cpu6502 *cpu) {
  uint8_t base = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t addr = base + cpu->Y;

  return (Operand) {
    .type = OPERAND_MEMORY,
    .addr = addr,
    .page_crossed = 0
  };
}

Operand addr_implied(cpu6502 *cpu __attribute__((unused))) {
  return (Operand) { 
    .type = OPERAND_MEMORY,
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
    .type = OPERAND_MEMORY,
    .addr = ((uint16_t)target_high << 8) | target_low,
    .page_crossed = 0
  };
}

Operand addr_rel(cpu6502 *cpu) {
  int8_t offset = (int8_t)cpu->read(cpu->ctx, cpu->PC++);
  uint16_t base = cpu->PC;
  uint16_t addr = base + offset;

  return (Operand) {
      .type = OPERAND_MEMORY,
      .addr = addr,
      .page_crossed = 0,
  };
}

Operand addr_acc(cpu6502 *cpu __attribute__((unused))) {
  return (Operand) {
    .type = OPERAND_ACCUMULATOR,
    .addr = 0,
    .page_crossed = 0,
  };
}

const AddressingMode addr_modes[13] = {
  [ADDR_ABS]    = { .type = ADDR_ABS,   .address = addr_abs,          .format = "$%04X",      .bytes = 2 },
  [ADDR_ABS_X]  = { .type = ADDR_ABS_X, .address = addr_abs_x,        .format = "$%04X,X",    .bytes = 2 },
  [ADDR_ABS_Y]  = { .type = ADDR_ABS_Y, .address = addr_abs_y,        .format = "$%04X,Y",    .bytes = 2 },
  [ADDR_ACC]    = { .type = ADDR_ACC,   .address = addr_acc,          .format = "A",          .bytes = 0 },
  [ADDR_IMM]    = { .type = ADDR_IMM,   .address = addr_imm,          .format = "#$%02X",     .bytes = 1 },
  [ADDR_IMP]    = { .type = ADDR_IMP,   .address = addr_implied,      .format = "",           .bytes = 0 },
  [ADDR_IND]    = { .type = ADDR_IND,   .address = addr_indirect,     .format = "($%04X)",    .bytes = 2 },
  [ADDR_IND_X]  = { .type = ADDR_IND_X, .address = addr_indirect_x,   .format = "($%02X,X)",  .bytes = 1 },
  [ADDR_IND_Y]  = { .type = ADDR_ABS_Y, .address = addr_indirect_y,   .format = "($%02X),Y",  .bytes = 1 },
  [ADDR_REL]    = { .type = ADDR_REL,   .address = addr_rel,          .format = "$%04X",      .bytes = 2 },
  [ADDR_ZP]     = { .type = ADDR_ZP,    .address = addr_zero_page,    .format = "$%02X",      .bytes = 1 },
  [ADDR_ZP_X]   = { .type = ADDR_ZP_X,  .address = addr_zero_page_x,  .format = "$%02X,X",    .bytes = 1 },
  [ADDR_ZP_Y]   = { .type = ADDR_ZP_Y,  .address = addr_zero_page_y,  .format = "$%02X,Y",    .bytes = 1 }
};