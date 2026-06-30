#ifndef TRACE_H
#define TRACE_H

#include <lib6502/6502.h>

#include "addressing.h"
#include "instructions.h"

void build_trace(CPU6502Trace* t, CPU6502* cpu, uint16_t initial_pc,
                 AddressingMode* addr_mode, Instruction* instruction);

#endif