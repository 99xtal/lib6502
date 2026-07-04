#include "micro_ops.h"

#include <string.h>

#include "6502.h"
#include "flags.h"

void finish_op(CPU6502* cpu) { memset(&cpu->op, 0, sizeof(cpu->op)); }

void dummy(CPU6502* cpu) { (void)cpu; }

void dec_sp(CPU6502* cpu) { cpu->SP--; }

void read_reset_low(CPU6502* cpu) {
  cpu->op.addr_lo = cpu->read(cpu->ctx, 0xFFFC);
}

void read_reset_high_finish(CPU6502* cpu) {
  cpu->op.addr_hi = cpu->read(cpu->ctx, 0xFFFD);
  uint16_t addr = (cpu->op.addr_hi << 8) | cpu->op.addr_lo;
  cpu->PC = addr;
  cpu->status |= FLAG_INTERRUPT_DISABLE;

  cpu->reset_requested = false;
  finish_op(cpu);
}

const OpDef reset_sequence = {.name = "RESET",
                              .micro_ops = {
                                  dummy,
                                  dummy,
                                  dec_sp,
                                  dec_sp,
                                  dec_sp,
                                  read_reset_low,
                                  read_reset_high_finish,
                              }};