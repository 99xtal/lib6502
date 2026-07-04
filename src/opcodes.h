#ifndef OPCODES_H
#define OPCODES_H

#include <lib6502/6502.h>
#include <stdint.h>

#include "addressing.h"
#include "instructions.h"
#include "operand.h"

typedef int (*exec_fn)(CPU6502* cpu, Operand op);
typedef struct {
  Inst instruction;
  AddrMode addr_mode;
  uint8_t cycles;
  int page_cross_penalty;
} Opcode;

extern const Opcode opcode_table_nmos[256];

#endif  // OPCODES_H