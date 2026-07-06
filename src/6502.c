#include "6502.h"

#include <lib6502/6502.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "flags.h"
#include "micro_ops.h"
#include "stack.h"
#include "vectors.h"

int cpu6502_init(CPU6502* cpu, CPU6502Variant variant, CPU6502ReadFn read,
                 CPU6502WriteFn write, void* ctx) {
  if (!cpu) {
    return -1;
  }

  memset(cpu, 0, sizeof(*cpu));
  cpu->status = 0 | FLAG_UNUSED;  // Set unused flag to 1
  cpu->variant = variant;

  cpu->read = read;
  cpu->write = write;
  cpu->ctx = ctx;

  return 0;
}

CPU6502* cpu6502_create(CPU6502Variant variant, CPU6502ReadFn read,
                        CPU6502WriteFn write, void* ctx) {
  CPU6502* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  if (cpu6502_init(cpu, variant, read, write, ctx) != 0) {
    free(cpu);
    return NULL;
  }

  return cpu;
}

void cpu6502_reset(CPU6502* cpu) { cpu->reset_requested = true; }

void cpu6502_tick(CPU6502* cpu) {
  if (cpu->jammed) {
    return;
  }

  if (cpu->op.def == NULL) {
    if (cpu->reset_requested) {
      cpu->op.def = &reset_sequence;

      cpu->reset_requested = false;
    } else {
      uint8_t opcode = cpu->read(cpu->ctx, cpu->PC++);
      cpu->op.def = &instruction_defs[opcode];

      return;
    }
  }

  MicroOpFn micro_op = cpu->op.def->micro_ops[cpu->op.cycle++];
  micro_op(cpu);
}

int cpu6502_step(CPU6502* cpu) {
  int cycles = 0;

  do {
    cpu6502_tick(cpu);
    cycles++;
  } while (cpu->op.def != NULL && !cpu->jammed);

  return cycles;
}

int cpu6502_nmi(CPU6502* cpu) {
  stack_push_u16(cpu, cpu->PC);
  stack_push_u8(cpu, (cpu->status & ~FLAG_BREAK) | FLAG_UNUSED);

  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);

  cpu->PC = read_vector(cpu, VECTOR_NMI);

  return 7;
}

void cpu6502_destroy(CPU6502* cpu) { free(cpu); }

CPU6502State cpu6502_get_state(CPU6502* cpu) {
  return (CPU6502State){
      .A = cpu->A,
      .X = cpu->X,
      .Y = cpu->Y,
      .status = cpu->status,
      .PC = cpu->PC,
      .SP = cpu->SP,
  };
}

int cpu6502_set_pc(CPU6502* cpu, uint16_t addr) {
  if (!cpu) {
    return -1;
  }

  cpu->PC = addr;
  return 0;
}