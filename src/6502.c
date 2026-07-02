#include <lib6502/6502.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "addressing.h"
#include "flags.h"
#include "opcodes.h"
#include "stack.h"
#include "trace.h"
#include "vectors.h"

const Opcode* get_opcode_table(CPU6502Variant variant);
void format_disasm_operand(char* buf, size_t size, AddrMode mode,
                           uint16_t instr_addr, const uint8_t bytes[3]);

void cpu6502_init(CPU6502* cpu, CPU6502Variant variant, CPU6502ReadFn read,
                  CPU6502WriteFn write, void* ctx) {
  cpu->A = 0;
  cpu->X = 0;
  cpu->Y = 0;
  cpu->SP = 0;
  cpu->status = 0 | FLAG_UNUSED;  // Set unused flag to 1
  cpu->variant = variant;

  cpu->read = read;
  cpu->write = write;
  cpu->ctx = ctx;
}

int cpu6502_reset(CPU6502* cpu) {
  int clock_cycles = 7;
  uint16_t reset_position = read_vector(cpu, VECTOR_RESET);

  cpu->SP = cpu->SP - 3;
  cpu->PC = reset_position;

  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);

  return clock_cycles;
}

int cpu6502_step(CPU6502* cpu) {
  if (cpu->jammed) {
    return 1;  // burn a cycle
  }

  const Opcode* opcode_table = get_opcode_table(cpu->variant);

  uint16_t initial_pc = cpu->PC;

  // read opcode
  uint8_t opcode_byte = cpu->read(cpu->ctx, cpu->PC++);
  Opcode opcode = opcode_table[opcode_byte];
  AddressingMode addressing_mode = addr_modes[opcode.addr_mode];
  Instruction instruction = instructions[opcode.instruction];

  Operand op = addressing_mode.address(cpu);

  // build trace
  CPU6502Trace t = {0};
  if (cpu->trace) {
    build_trace(&t, cpu, initial_pc, &addressing_mode, &op, &instruction);
  }

  // execute instruction
  int extra_cycles = instruction.execute(cpu, op);

  int cycles = opcode.cycles + extra_cycles;

  if (op.page_crossed) {
    cycles += opcode.page_cross_penalty;
  }

  if (cpu->trace) {
    t.cycles = cycles;

    cpu->trace(cpu->trace_ctx, t);
  }

  return cycles;
}

int cpu6502_nmi(CPU6502* cpu) {
  stack_push_u16(cpu, cpu->PC);
  stack_push_u8(cpu, (cpu->status & ~FLAG_BREAK) | FLAG_UNUSED);

  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);

  cpu->PC = read_vector(cpu, VECTOR_NMI);

  return 7;
}

const Opcode* get_opcode_table(CPU6502Variant variant) {
  switch (variant) {
    case CPU6502_VARIANT_NMOS:
    case CPU6502_VARIANT_RP2A03:
      return opcode_table_nmos;
    case CPU6502_VARIANT_STRICT:
    default:
      return opcode_table_strict;
  }
}

bool cpu6502_disasm_at(CPU6502* cpu, uint16_t addr, CPU6502DisasmLine* out) {
  if (!cpu || !out) {
    return false;
  }

  uint8_t opcode_byte = cpu->read(cpu->ctx, addr);

  const Opcode* opcode_table = get_opcode_table(cpu->variant);
  Opcode opcode = opcode_table[opcode_byte];
  AddressingMode addressing_mode = addr_modes[opcode.addr_mode];
  Instruction instruction = instructions[opcode.instruction];

  out->addr = addr;
  out->mnemonic = instruction.mnemonic;
  out->bytes_count = addressing_mode.bytes + 1;

  out->bytes[0] = opcode_byte;
  for (uint8_t i = 0; i < addressing_mode.bytes; i++) {
    out->bytes[i + 1] = cpu->read(cpu->ctx, addr + i + 1);
  }

  format_disasm_operand(out->operand, sizeof(out->operand),
                        addressing_mode.type, addr, out->bytes);

  return true;
}

void format_disasm_operand(char* buf, size_t size, AddrMode mode,
                           uint16_t instr_addr, const uint8_t bytes[3]) {
  uint8_t lo = bytes[1];
  uint8_t hi = bytes[2];
  uint16_t abs = (uint16_t)lo | ((uint16_t)hi << 8);

  switch (mode) {
    case ADDR_IMP:
      snprintf(buf, size, "");
      break;

    case ADDR_ACC:
      snprintf(buf, size, "A");
      break;

    case ADDR_IMM:
      snprintf(buf, size, "#$%02X", lo);
      break;

    case ADDR_ZP:
      snprintf(buf, size, "$%02X", lo);
      break;

    case ADDR_ZP_X:
      snprintf(buf, size, "$%02X,X", lo);
      break;

    case ADDR_ZP_Y:
      snprintf(buf, size, "$%02X,Y", lo);
      break;

    case ADDR_ABS:
      snprintf(buf, size, "$%04X", abs);
      break;

    case ADDR_ABS_X:
      snprintf(buf, size, "$%04X,X", abs);
      break;

    case ADDR_ABS_Y:
      snprintf(buf, size, "$%04X,Y", abs);
      break;

    case ADDR_IND:
      snprintf(buf, size, "($%04X)", abs);
      break;

    case ADDR_IND_X:
      snprintf(buf, size, "($%02X,X)", lo);
      break;

    case ADDR_IND_Y:
      snprintf(buf, size, "($%02X),Y", lo);
      break;

    case ADDR_REL: {
      int8_t offset = (int8_t)lo;
      uint16_t target = instr_addr + 2 + offset;
      snprintf(buf, size, "$%04X", target);
      break;
    }

    default:
      snprintf(buf, size, "???");
      break;
  }
}