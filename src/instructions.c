#include <lib6502/6502.h>

#include "flags.h"
#include "instructions.h"
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

int tax(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->X = cpu->A;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->X & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->X == 0 ? 1 : 0);

  return 0;
}

int tay(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->Y = cpu->A;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->Y & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->Y == 0 ? 1 : 0);

  return 0;
}

int txa(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->A = cpu->X;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int tya(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->A = cpu->Y;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int nop(cpu6502 *cpu __attribute__((unused)), Operand op __attribute__((unused))) {
  return 0;
}

int tsx(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->X = cpu->SP;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->X & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->X == 0 ? 1 : 0);

  return 0;
}

int txs(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->SP = cpu->X;

  return 0;
}

int pha(cpu6502 *cpu, Operand op __attribute__((unused))) {
  stack_push_u8(cpu, cpu->A);

  return 0;
}

int php(cpu6502 *cpu, Operand op __attribute__((unused))) {
  uint8_t status = cpu->status;
  status |= FLAG_BREAK;
  status |= FLAG_UNUSED;   // bit 5 is usually always pushed as 1
  
  stack_push_u8(cpu, status);

  return 0;
}

int pla(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->A = stack_pop_u8(cpu);

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int plp(cpu6502 *cpu, Operand op __attribute__((unused)) ) {
  uint8_t status_value = stack_pop_u8(cpu);

  cpu->status = (status_value & ~FLAG_BREAK) | FLAG_UNUSED;

  return 0;
}

void execute_adc(cpu6502 *cpu, uint8_t value) {
  uint8_t carry_in = get_flag(cpu, FLAG_CARRY);

  if (get_flag(cpu, FLAG_DECIMAL_MODE) && cpu->variant != CPU6502_VARIANT_RP2A03) {
    uint16_t low_nibble = (cpu->A & 0x0F) + (value & 0x0F) + carry_in;
    if (low_nibble > 0x09) {
      low_nibble += 0x06;
    }

    uint16_t carry_to_high = (low_nibble >> 4) & 0x01; 
    uint16_t high_nibble = (cpu->A & 0xF0) + (value & 0xF0) + (carry_to_high << 4);

    if (high_nibble > 0x9F) {
      high_nibble += 0x60;
    }

    set_flag(cpu, FLAG_CARRY, high_nibble > 0xFF);

    uint16_t decimal_sum = (high_nibble & 0xF0) | (low_nibble & 0x0F);
    uint8_t decimal_result = (uint8_t)decimal_sum;

    cpu->A = decimal_result;

  } else {
    uint16_t binary_sum = (uint16_t)cpu->A + value + carry_in;
    uint8_t binary_result = (uint8_t)binary_sum;

    set_flag(cpu, FLAG_CARRY, binary_sum > 0xFF);
    set_flag(cpu, FLAG_ZERO, binary_result == 0);
    set_flag(cpu, FLAG_NEGATIVE, (binary_result & 0x80) != 0);
      // Overflow happens when A and value have the same sign,
    // but the result has a different sign.
    set_flag(cpu, FLAG_OVERFLOW,
        (~(cpu->A ^ value) & (cpu->A ^ binary_result) & 0x80) != 0
    );
    cpu->A = binary_result;
  }
}

int adc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  execute_adc(cpu, value);

  return 0;
}

void execute_sbc(cpu6502 *cpu, uint8_t value) {
  uint8_t carry = get_flag(cpu, FLAG_CARRY) ? 1 : 0;
  uint8_t old_a = cpu->A;

  uint16_t binary_diff = (uint16_t)old_a + (uint8_t)(~value) + carry;
  uint8_t binary_result = (uint8_t)binary_diff;

  set_flag(cpu, FLAG_CARRY, binary_diff > 0xFF);
  set_flag(cpu, FLAG_ZERO, binary_result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (binary_result & 0x80) != 0);
  set_flag(cpu, FLAG_OVERFLOW,
    ((old_a ^ value) & (old_a ^ binary_result) & 0x80) != 0
  );

  if (get_flag(cpu, FLAG_DECIMAL_MODE) && cpu->variant != CPU6502_VARIANT_RP2A03) {
    int16_t al = (old_a & 0x0F) - (value & 0x0F) - (1 - carry);
    int16_t ah = (old_a >> 4) - (value >> 4);

    if (al < 0) {
      al -= 6;
      ah -= 1;
    }

    if (ah < 0) {
      ah -= 6;
    }

    cpu->A = (uint8_t)(((ah << 4) & 0xF0) | (al & 0x0F));
  } else {
    cpu->A = binary_result;
  }
}

int sbc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  execute_sbc(cpu, value);

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

int jsr(cpu6502 *cpu, Operand op) {
  uint16_t return_addr = cpu->PC - 1;
  stack_push_u16(cpu, return_addr);

  cpu->PC = op.addr;

  return 0;
}

int rts(cpu6502 *cpu, Operand op __attribute__((unused))) {
  uint16_t return_addr = stack_pop_u16(cpu) + 1;
  cpu->PC = return_addr;

  return 0;
}

int inc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t result = value + 1;
  cpu->write(cpu->ctx, op.addr, result);

  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int inx(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->X++;

  set_flag(cpu, FLAG_ZERO, cpu->X == 0);
  set_flag(cpu, FLAG_NEGATIVE, (cpu->X & 0x80) != 0);
  
  return 0;
}

int iny(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->Y++;

  set_flag(cpu, FLAG_ZERO, cpu->Y == 0);
  set_flag(cpu, FLAG_NEGATIVE, (cpu->Y & 0x80) != 0);
  
  return 0;
}

int dec(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t result = value - 1;
  cpu->write(cpu->ctx, op.addr, result);

  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int dex(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->X--;

  set_flag(cpu, FLAG_ZERO, cpu->X == 0);
  set_flag(cpu, FLAG_NEGATIVE, (cpu->X & 0x80) != 0);
  
  return 0;
}

int dey(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->Y--;

  set_flag(cpu, FLAG_ZERO, cpu->Y == 0);
  set_flag(cpu, FLAG_NEGATIVE, (cpu->Y & 0x80) != 0);
  
  return 0;
}

int branch(cpu6502 *cpu, Operand op, int condition) {
  if(!condition) {
    return 0;
  }

  uint16_t old_pc = cpu->PC;
  cpu->PC = op.addr;

  // detect page cross
  if ((old_pc & 0xFF00) != (cpu->PC & 0xFF00)) {
    return 2;
  }

  return 1;
}

int bcc(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, !get_flag(cpu, FLAG_CARRY));
}

int bcs(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, get_flag(cpu, FLAG_CARRY));
}

int beq(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, get_flag(cpu, FLAG_ZERO));
}

int bmi(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, get_flag(cpu, FLAG_NEGATIVE));
}

int bne(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, !get_flag(cpu, FLAG_ZERO));
}

int bpl(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, !get_flag(cpu, FLAG_NEGATIVE));
}

int bvc(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, !get_flag(cpu, FLAG_OVERFLOW));
}

int bvs(cpu6502 *cpu, Operand op) {
  return branch(cpu, op, get_flag(cpu, FLAG_OVERFLOW));
}

int asl(cpu6502 *cpu, Operand op) {
  uint8_t value = op.type == OPERAND_MEMORY
    ? cpu->read(cpu->ctx, op.addr)
    : cpu->A;

  uint8_t result = value << 1;
  uint8_t last_bit = (value & 0x80) != 0;

  if (op.type == OPERAND_MEMORY) {
    cpu->write(cpu->ctx, op.addr, result);
  } else {
    cpu->A = result;
  }

  set_flag(cpu, FLAG_CARRY, last_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int lsr(cpu6502 *cpu, Operand op) {
  uint8_t value = op.type == OPERAND_MEMORY
    ? cpu->read(cpu->ctx, op.addr)
    : cpu->A;
  
  uint8_t result = value >> 1;
  uint8_t first_bit = (value & 0x01) != 0;

  if (op.type == OPERAND_MEMORY) {
    cpu->write(cpu->ctx, op.addr, result);
  } else {
    cpu->A = result;
  }

  set_flag(cpu, FLAG_CARRY, first_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, 0);

  return 0;
}

int rol(cpu6502 *cpu, Operand op) {
  uint8_t value = op.type == OPERAND_MEMORY
    ? cpu->read(cpu->ctx, op.addr)
    : cpu->A;

  uint8_t result = value << 1;
  uint8_t old_last_bit = (value & 0x80) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit;

  if (op.type == OPERAND_MEMORY) {
    cpu->write(cpu->ctx, op.addr, result);
  } else {
    cpu->A = result;
  }

  set_flag(cpu, FLAG_CARRY, old_last_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int ror(cpu6502 *cpu, Operand op) {
  uint8_t value = op.type == OPERAND_MEMORY
    ? cpu->read(cpu->ctx, op.addr)
    : cpu->A;

  uint8_t result = value >> 1;
  uint8_t old_first_bit = (value & 0x01) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit << 7;

  if (op.type == OPERAND_MEMORY) {
    cpu->write(cpu->ctx, op.addr, result);
  } else {
    cpu->A = result;
  }

  set_flag(cpu, FLAG_CARRY, old_first_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  return 0;
}

int kil(cpu6502 *cpu, Operand op __attribute__((unused))) {
  cpu->jammed = true;
  return 0;
}

int slo(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  set_flag(cpu, FLAG_CARRY, value & 0x80);

  value <<= 1;
  cpu->write(cpu->ctx, op.addr, value);

  cpu->A |= value;

  set_flag(cpu, FLAG_ZERO, cpu->A == 0);
  set_flag(cpu, FLAG_NEGATIVE, cpu->A & 0x80);

  return 0;
}

int anc(cpu6502 *cpu, Operand op) {
  // AND operation
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A &= value;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  // set carry as if it were ASL
  uint8_t last_bit = (value & 0x80) != 0;
  set_flag(cpu, FLAG_CARRY, last_bit);

  return 0;
}

int rla(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  uint8_t old_carry = get_flag(cpu, FLAG_CARRY) ? 1 : 0;
  uint8_t rol_carry = (value & 0x80) != 0;

  uint8_t result = value << 1;
  result |= old_carry;

  cpu->write(cpu->ctx, op.addr, result);

  set_flag(cpu, FLAG_CARRY, rol_carry);

  // AND rotated memory into A
  cpu->A &= result;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0);

  return 0;
}

int sre(cpu6502 *cpu, Operand op) {
  // LSR
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  
  uint8_t result = value >> 1;
  uint8_t first_bit = (value & 0x01) != 0;

  cpu->write(cpu->ctx, op.addr, result);

  set_flag(cpu, FLAG_CARRY, first_bit);

  // EOR
  cpu->A ^= result;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int alr(cpu6502 *cpu, Operand op) {
  // AND
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A &= value;

  // LSR
  uint8_t carry = cpu->A & 0x01;
  uint8_t result = cpu->A >> 1;

  cpu->A = result;

  set_flag(cpu, FLAG_CARRY, carry);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, 0);

  return 0;
}

int rra(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  uint8_t result = value >> 1;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit << 7;

  set_flag(cpu, FLAG_CARRY, value & 0x01);   // ROR carry-out
  
  cpu->write(cpu->ctx, op.addr, result);

  execute_adc(cpu, result);

  return 0;
}

int arr(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  cpu->A &= value;

  uint8_t carry_in = get_flag(cpu, FLAG_CARRY);

  cpu->A = (cpu->A >> 1) | (carry_in << 7);

  set_flag(cpu, FLAG_CARRY, cpu->A & 0x80);
  set_flag(cpu, FLAG_OVERFLOW,
      ((cpu->A >> 6) ^ (cpu->A >> 5)) & 1);

  set_flag(cpu, FLAG_ZERO, cpu->A == 0);
  set_flag(cpu, FLAG_NEGATIVE, cpu->A & 0x80);

  return 0;
}

int sax(cpu6502 *cpu, Operand op) {
  uint8_t result = cpu->A & cpu->X;

  cpu->write(cpu->ctx, op.addr, result);

  return 0;
}

int xaa(cpu6502 *cpu, Operand op) {
  cpu->A = cpu->X;

  uint8_t value = cpu->read(cpu->ctx, op.addr);
  cpu->A &= value;

  set_flag(cpu, FLAG_NEGATIVE, (cpu->A & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, cpu->A == 0 ? 1 : 0);

  return 0;
}

int ahx(cpu6502 *cpu, Operand op) {
  uint8_t high = (uint8_t)((op.addr + 1) >> 8);
  uint8_t result = cpu->A & cpu->X & high;

  cpu->write(cpu->ctx, op.addr, result);

  return 0;
}

int tas(cpu6502 *cpu, Operand op) {
  cpu->SP = cpu->A & cpu->X;
  uint8_t high = (uint8_t)((op.addr + 1) >> 8);
  
  cpu->write(cpu->ctx, op.addr, cpu->SP & high);

  return 0;
}

int shy(cpu6502 *cpu, Operand op) {
  uint8_t high = (uint8_t)((op.addr + 1) >> 8);

  cpu->write(cpu->ctx, op.addr, cpu->Y & high);

  return 0;
}

int shx(cpu6502 *cpu, Operand op) {
  uint8_t high = (uint8_t)((op.addr + 1) >> 8);

  cpu->write(cpu->ctx, op.addr, cpu->X & high);

  return 0;
}

int lax(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);

  cpu->A = value;
  cpu->X = value;

  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, value == 0 ? 1 : 0);

  return 0;
}

int las(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr) & cpu->SP;

  cpu->A = value;
  cpu->X = value;
  cpu->SP = value;

  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, value == 0 ? 1 : 0);

  return 0;
}

int dcp(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t dec_result = value - 1;
  cpu->write(cpu->ctx, op.addr, dec_result);

  uint8_t cmp_result = (uint16_t)cpu->A - dec_result;

  set_flag(cpu, FLAG_CARRY, cpu->A >= dec_result);
  set_flag(cpu, FLAG_ZERO, cpu->A == dec_result);
  set_flag(cpu, FLAG_NEGATIVE, (cmp_result & 0x80) != 0);

  return 0;
}

int axs(cpu6502 *cpu, Operand op) {
  cpu->write(cpu->ctx, op.addr, cpu->A & cpu->X);

  return 0;
}

int isc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
  uint8_t result = value + 1;
  cpu->write(cpu->ctx, op.addr, result);

  execute_sbc(cpu, result);

  return 0;
}

const Instruction instructions[] = {
  /* Load/Store Operations */
  [INST_LDA] = { .mnemonic = "LDA", .execute = lda , .is_undocumented = 0 },
  [INST_LDX] = { .mnemonic = "LDX", .execute = ldx , .is_undocumented = 0 },
  [INST_LDY] = { .mnemonic = "LDY", .execute = ldy , .is_undocumented = 0 },
  [INST_STA] = { .mnemonic = "STA", .execute = sta , .is_undocumented = 0 },
  [INST_STX] = { .mnemonic = "STX", .execute = stx , .is_undocumented = 0 },
  [INST_STY] = { .mnemonic = "STY", .execute = sty , .is_undocumented = 0 },

  /* Register Transfers */
  [INST_TAX] = { .mnemonic = "TAX", .execute = tax , .is_undocumented = 0 },
  [INST_TAY] = { .mnemonic = "TAY", .execute = tay , .is_undocumented = 0 },
  [INST_TXA] = { .mnemonic = "TXA", .execute = txa , .is_undocumented = 0 },
  [INST_TYA] = { .mnemonic = "TYA", .execute = tya , .is_undocumented = 0 },

  /* Stack Operations */
  [INST_TSX] = { .mnemonic = "TSX", .execute = tsx , .is_undocumented = 0 },
  [INST_TXS] = { .mnemonic = "TXS", .execute = txs , .is_undocumented = 0 },
  [INST_PHA] = { .mnemonic = "PHA", .execute = pha , .is_undocumented = 0 },
  [INST_PHP] = { .mnemonic = "PHP", .execute = php , .is_undocumented = 0 },
  [INST_PLA] = { .mnemonic = "PLA", .execute = pla , .is_undocumented = 0 },
  [INST_PLP] = { .mnemonic = "PLP", .execute = plp , .is_undocumented = 0 },

  /* Logical */
  [INST_AND] = { .mnemonic = "AND", .execute = and , .is_undocumented = 0 },
  [INST_EOR] = { .mnemonic = "EOR", .execute = eor , .is_undocumented = 0 },
  [INST_ORA] = { .mnemonic = "ORA", .execute = ora , .is_undocumented = 0 },
  [INST_BIT] = { .mnemonic = "BIT", .execute = bit , .is_undocumented = 0 },

  /* Arithmetic */
  [INST_ADC] = { .mnemonic = "ADC", .execute = adc , .is_undocumented = 0 },
  [INST_SBC] = { .mnemonic = "SBC", .execute = sbc , .is_undocumented = 0 },
  [INST_CMP] = { .mnemonic = "CMP", .execute = cmp , .is_undocumented = 0 },
  [INST_CPX] = { .mnemonic = "CPX", .execute = cpx , .is_undocumented = 0 },
  [INST_CPY] = { .mnemonic = "CPY", .execute = cpy , .is_undocumented = 0 },

  /* Increments & Decrements */
  [INST_INC] = { .mnemonic = "INC", .execute = inc , .is_undocumented = 0 },
  [INST_INX] = { .mnemonic = "INX", .execute = inx , .is_undocumented = 0 },
  [INST_INY] = { .mnemonic = "INY", .execute = iny , .is_undocumented = 0 },
  [INST_DEC] = { .mnemonic = "DEC", .execute = dec , .is_undocumented = 0 },
  [INST_DEX] = { .mnemonic = "DEX", .execute = dex , .is_undocumented = 0 },
  [INST_DEY] = { .mnemonic = "DEY", .execute = dey , .is_undocumented = 0 },

  /* Shifts */
  [INST_ASL] = { .mnemonic = "ASL", .execute = asl , .is_undocumented = 0 },
  [INST_LSR] = { .mnemonic = "LSR", .execute = lsr , .is_undocumented = 0 },
  [INST_ROL] = { .mnemonic = "ROL", .execute = rol , .is_undocumented = 0 },
  [INST_ROR] = { .mnemonic = "ROR", .execute = ror , .is_undocumented = 0 },

  /* Jumps & Calls */
  [INST_JMP] = { .mnemonic = "JMP", .execute = jmp , .is_undocumented = 0 },
  [INST_JSR] = { .mnemonic = "JSR", .execute = jsr , .is_undocumented = 0 },
  [INST_RTS] = { .mnemonic = "RTS", .execute = rts , .is_undocumented = 0 },

  /* Branches */
  [INST_BCC] = { .mnemonic = "BCC", .execute = bcc , .is_undocumented = 0 },
  [INST_BCS] = { .mnemonic = "BCS", .execute = bcs , .is_undocumented = 0 },
  [INST_BEQ] = { .mnemonic = "BEQ", .execute = beq , .is_undocumented = 0 },
  [INST_BMI] = { .mnemonic = "BMI", .execute = bmi , .is_undocumented = 0 },
  [INST_BNE] = { .mnemonic = "BNE", .execute = bne , .is_undocumented = 0 },
  [INST_BPL] = { .mnemonic = "BPL", .execute = bpl , .is_undocumented = 0 },
  [INST_BVC] = { .mnemonic = "BVC", .execute = bvc , .is_undocumented = 0 },
  [INST_BVS] = { .mnemonic = "BVS", .execute = bvs , .is_undocumented = 0 },

  /* Status Flag Changes */
  [INST_CLC] = { .mnemonic = "CLC", .execute = clc , .is_undocumented = 0 },
  [INST_CLD] = { .mnemonic = "CLD", .execute = cld , .is_undocumented = 0 },
  [INST_CLI] = { .mnemonic = "CLI", .execute = cli , .is_undocumented = 0 },
  [INST_CLV] = { .mnemonic = "CLV", .execute = clv , .is_undocumented = 0 },
  [INST_SEC] = { .mnemonic = "SEC", .execute = sec , .is_undocumented = 0 },
  [INST_SED] = { .mnemonic = "SED", .execute = sed , .is_undocumented = 0 },
  [INST_SEI] = { .mnemonic = "SEI", .execute = sei , .is_undocumented = 0 },

  /* System Functions */
  [INST_BRK] = { .mnemonic = "BRK", .execute = brk , .is_undocumented = 0 },
  [INST_NOP] = { .mnemonic = "NOP", .execute = nop , .is_undocumented = 0 },
  [INST_RTI] = { .mnemonic = "RTI", .execute = rti , .is_undocumented = 0 },

  /* Undocumented Instructions */
  [INST_KIL] = { .mnemonic = "KIL", .execute = kil , .is_undocumented = 1 },
  [INST_SLO] = { .mnemonic = "SLO", .execute = slo , .is_undocumented = 1 },
  [INST_ANC] = { .mnemonic = "ANC", .execute = anc , .is_undocumented = 1 },
  [INST_RLA] = { .mnemonic = "RLA", .execute = rla , .is_undocumented = 1 },
  [INST_SRE] = { .mnemonic = "SRE", .execute = sre , .is_undocumented = 1 },
  [INST_ALR] = { .mnemonic = "ALR", .execute = alr , .is_undocumented = 1 },
  [INST_RRA] = { .mnemonic = "RRA", .execute = rra , .is_undocumented = 1 },
  [INST_ARR] = { .mnemonic = "ARR", .execute = arr , .is_undocumented = 1 },
  [INST_SAX] = { .mnemonic = "SAX", .execute = sax , .is_undocumented = 1 },
  [INST_XAA] = { .mnemonic = "XAA", .execute = xaa , .is_undocumented = 1 },
  [INST_AHX] = { .mnemonic = "AHX", .execute = ahx , .is_undocumented = 1 },
  [INST_TAS] = { .mnemonic = "TAS", .execute = tas , .is_undocumented = 1 },
  [INST_SHY] = { .mnemonic = "SHY", .execute = shy , .is_undocumented = 1 },
  [INST_SHX] = { .mnemonic = "SHX", .execute = shx , .is_undocumented = 1 },
  [INST_LAX] = { .mnemonic = "LAX", .execute = lax , .is_undocumented = 1 },
  [INST_LAS] = { .mnemonic = "LAS", .execute = las , .is_undocumented = 1 },
  [INST_DCP] = { .mnemonic = "DCP", .execute = dcp , .is_undocumented = 1 },
  [INST_AXS] = { .mnemonic = "AXS", .execute = axs , .is_undocumented = 1 },
  [INST_ISC] = { .mnemonic = "ISC", .execute = isc , .is_undocumented = 1 },
  [INST_NOP_UND] = { .mnemonic = "NOP", .execute = nop, .is_undocumented = 1 },
  [INST_SBC_UND] = { .mnemonic = "SBC", .execute = sbc, .is_undocumented = 1 },
};