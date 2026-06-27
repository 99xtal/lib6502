#ifndef TRACE_H
#define TRACE_H

#include <lib6502/6502.h>

#include "addressing.h"
#include "instructions.h"
#include "operand.h"

void build_trace(
  cpu6502_trace *t,
  cpu6502 *cpu,
  uint16_t initial_pc,
  Operand *op,
  AddressingMode *addr_mode,
  Instruction *instruction
);

#endif