#include <lib6502/6502.h>
#include "flags.h"
#include "opcodes.h"

int clc(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_CARRY, 0);

  return 0;
}

int cld(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_DECIMAL_MODE, 0);

  return 0;
}

int cli(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 0);

  return 0;
}

int clv(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_OVERFLOW, 0);

  return 0;
}

int sec(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_CARRY, 1);

  return 0;
}

int sed(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_DECIMAL_MODE, 1);

  return 0;
}

int sei(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);

  return 0;
}

int lda(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A = value;
  
  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, value == 0 ? 1 : 0);

  return 0;
}

int nop(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  return 0;
}