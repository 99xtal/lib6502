#include <stdio.h>
#include <string.h>

#include "trace.h"

void build_trace(
  cpu6502_trace *t,
  cpu6502 *cpu,
  uint16_t initial_pc,
  AddressingMode *addr_mode,
  Instruction *instruction
) {
  uint8_t bytes[3];

  uint8_t total_bytes = addr_mode->bytes + 1;

  for (uint8_t i = 0; i < total_bytes; i++) {
    uint16_t p = initial_pc + i;
    bytes[i] = cpu->read(cpu->ctx, p);
  }

  t->PC = initial_pc,
  t->bytes_count = total_bytes,
  t->mnemonic = instruction->mnemonic,
  t->A = cpu->A,
  t->X = cpu->X,
  t->Y = cpu->Y,
  t->status = cpu->status,
  t->SP = cpu->SP,
  memcpy(t->bytes, bytes, sizeof bytes);

  if (addr_mode->format && addr_mode->format[0] != '\0') {
    if (total_bytes == 2) {
      snprintf(t->operand, sizeof t->operand,
              addr_mode->format,
              bytes[1]);
    } else if (total_bytes == 3) {
      uint16_t operand =
          (uint16_t)bytes[1] |
          ((uint16_t)bytes[2] << 8);

      snprintf(t->operand, sizeof t->operand,
              addr_mode->format,
              operand);
    }
  }
}