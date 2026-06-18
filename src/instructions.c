#include <lib6502/6502.h>
#include "flags.h"
#include "opcodes.h"
#include "stack.h"
#include "vectors.h"

int and(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A &= value;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int eor(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A ^= value;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int ora(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A |= value;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int bit(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  set_flag(cpu, FLAG_ZERO, (cpu->A & value) == 0);
  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_OVERFLOW, (value & 0x40) != 0);

  return 0;
}

int brk(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->PC++;

  stack_push_u16(cpu, cpu->PC);

  uint8_t status = cpu->status;
  status |= FLAG_BREAK;
  status |= FLAG_UNUSED;   // bit 5 is usually always pushed as 1

  stack_push_u8(cpu, status);
  
  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);
  cpu->PC = read_vector(cpu, VECTOR_IRQ);

  return 0;
}

int rti(cpu6502 *cpu, Operand op) {
  (void)op;

  cpu->status = stack_pop_u8(cpu);

  /*
    Bit 5 is conventionally always treated as set in the status register.
    The break flag is not a real internal CPU latch in quite the same way,
    so many emulators normalize these bits after pulling P.
  */
  cpu->status &= ~FLAG_BREAK;
  cpu->status |= FLAG_UNUSED;

  cpu->PC = stack_pop_u16(cpu);

  return 0;
}

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

int ldx(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->X = value;

  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, value == 0 ? 1 : 0);

  return 0;
}

int ldy(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->Y = value;

  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, value == 0 ? 1 : 0);

  return 0;
}

int sta(cpu6502 *cpu, Operand op) {
  cpu->write(cpu->ctx, op.addr, cpu->A);

  return 0;
}

int stx(cpu6502 *cpu, Operand op) {
  cpu->write(cpu->ctx, op.addr, cpu->X);

  return 0;
}

int sty(cpu6502 *cpu, Operand op) {
  cpu->write(cpu->ctx, op.addr, cpu->Y);

  return 0;
}

int tax(cpu6502 *cpu, Operand op) {
  cpu->X = cpu->A;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->X & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->X == 0 ? 1 : 0);

  return 0;
}

int tay(cpu6502 *cpu, Operand op) {
  cpu->Y = cpu->A;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->Y & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->Y == 0 ? 1 : 0);

  return 0;
}

int txa(cpu6502 *cpu, Operand op) {
  cpu->A = cpu->X;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int tya(cpu6502 *cpu, Operand op) {
  cpu->A = cpu->Y;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int nop(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  return 0;
}

int tsx(cpu6502 *cpu, Operand op) {
  cpu->X = cpu->SP;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->X & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->X == 0 ? 1 : 0);

  return 0;
}

int txs(cpu6502 *cpu, Operand op) {
  cpu->SP = cpu->X;

  return 0;
}

int pha(cpu6502 *cpu, Operand op) {
  stack_push_u8(cpu, cpu->A);

  return 0;
}

int php(cpu6502 *cpu, Operand op) {
  stack_push_u8(cpu, cpu->status);

  return 0;
}

int pla(cpu6502 *cpu, Operand op) {
  cpu->A = stack_pop_u8(cpu);

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int plp(cpu6502 *cpu, Operand op) {
  cpu->status = stack_pop_u8(cpu);

  return 0;
}

int adc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t carry_in = get_flag(cpu, FLAG_CARRY);

  uint16_t sum = (uint16_t)cpu->A + value + carry_in;
  uint8_t result = (uint8_t)sum;

  set_flag(cpu, FLAG_CARRY, sum > 0xFF);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  // Overflow happens when A and value have the same sign,
  // but the result has a different sign.
  set_flag(cpu, FLAG_OVERFLOW,
      (~(cpu->A ^ value) & (cpu->A ^ result) & 0x80) != 0
  );

  cpu->A = result;

  return 0;
}

int sbc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t carry = get_flag(cpu, FLAG_CARRY);

  uint16_t result = (uint16_t)cpu->A + (uint8_t)(~value) + carry;
  uint8_t final = (uint8_t)result;

  set_flag(cpu, FLAG_CARRY, result > 0xFF);
  set_flag(cpu, FLAG_ZERO, final == 0);
  set_flag(cpu, FLAG_NEGATIVE, (final & 0x80) != 0);

  // Overflow happens when A and value have the same sign,
  // but the result has a different sign.
  set_flag(cpu, FLAG_OVERFLOW,
      (~(cpu->A ^ value) & (cpu->A ^ final) & 0x80) != 0
  );

  cpu->A = result;

  return 0;
}

int cmp(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t result = (uint16_t)cpu->A - value;

  set_flag(cpu, FLAG_CARRY, cpu->A >= value);
  set_flag(cpu, FLAG_ZERO, cpu->A == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int cpx(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t result = (uint16_t)cpu->X - value;

  set_flag(cpu, FLAG_CARRY, cpu->X >= value);
  set_flag(cpu, FLAG_ZERO, cpu->X == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int cpy(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t result = (uint16_t)cpu->Y - value;

  set_flag(cpu, FLAG_CARRY, cpu->Y >= value);
  set_flag(cpu, FLAG_ZERO, cpu->Y == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int jmp(cpu6502 *cpu, Operand op) {
  cpu->PC = op.addr;

  return 0;
}