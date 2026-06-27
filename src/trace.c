#include <stdio.h>
#include <string.h>

#include "trace.h"

void format_operand(char *trace_operand, size_t trace_operand_size, cpu6502 *cpu, AddressingMode *addr_mode, Operand *op, int initial_pc);

void build_trace(
  cpu6502_trace *t,
  cpu6502 *cpu,
  uint16_t initial_pc,
  Operand *op,
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
    format_operand(t->operand, sizeof t->operand, cpu, addr_mode, op, initial_pc);
  }
}

void format_operand(char *trace_operand, size_t trace_operand_size, cpu6502 *cpu, AddressingMode *addr_mode, Operand *op, int initial_pc) {
  uint8_t bytes[3];
  uint8_t total_bytes = addr_mode->bytes + 1;

  for (uint8_t i = 0; i < total_bytes; i++) {
    uint16_t p = initial_pc + i;
    bytes[i] = cpu->read(cpu->ctx, p);
  }

  uint16_t raw16 = (uint16_t)bytes[1] | ((uint16_t)bytes[2] << 8);

  switch (addr_mode->type) {
    case ADDR_ZP:
      snprintf(trace_operand, trace_operand_size, "$%02X = %02X",
              bytes[1],
              cpu->read(cpu->ctx, op->addr)
      );
      break;
    case ADDR_REL:
      snprintf(trace_operand, trace_operand_size, addr_mode->format, op->addr);
      break;
    default:
      if (total_bytes == 2) {
        snprintf(trace_operand, trace_operand_size, 
            addr_mode->format,
            bytes[1]);
      } else if (total_bytes == 3) {
        snprintf(trace_operand, trace_operand_size, 
            addr_mode->format,
            raw16);
      }
    break;
  }
}