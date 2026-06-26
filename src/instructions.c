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
  cpu->status = stack_pop_u8(cpu);

  return 0;
}

void execute_adc(cpu6502 *cpu, uint8_t value) {
  uint8_t carry_in = get_flag(cpu, FLAG_CARRY);

  if (get_flag(cpu, FLAG_DECIMAL_MODE)) {
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

int sbc(cpu6502 *cpu, Operand op) {
  uint8_t value = cpu->read(cpu->ctx, op.addr);
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

  if (get_flag(cpu, FLAG_DECIMAL_MODE)) {
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
  uint8_t raw = cpu->read(cpu->ctx, op.addr);
  int8_t offset = (int8_t)raw;

  if(!condition) {
    return 0;
  }

  uint16_t old_pc = cpu->PC;
  cpu->PC += offset;

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
}

int sax(cpu6502 *cpu, Operand op) {
  uint8_t result = cpu->A & cpu->X;

  cpu->write(cpu->ctx, op.addr, result);
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
  uint8_t result = cpu->A & cpu->X & (uint8_t)((op.addr & 0XFF00) >> 8);

  cpu->write(cpu->ctx, op.addr, result);
}

const Instruction instructions[] = {
  /* Load/Store Operations */
  [INST_LDA] = { .mnemonic = "LDA", .execute = lda },
  [INST_LDX] = { .mnemonic = "LDX", .execute = ldx },
  [INST_LDY] = { .mnemonic = "LDY", .execute = ldy },
  [INST_STA] = { .mnemonic = "STA", .execute = sta },
  [INST_STX] = { .mnemonic = "STX", .execute = stx },
  [INST_STY] = { .mnemonic = "STY", .execute = sty },

  /* Register Transfers */
  [INST_TAX] = { .mnemonic = "TAX", .execute = tax },
  [INST_TAY] = { .mnemonic = "TAY", .execute = tay },
  [INST_TXA] = { .mnemonic = "TXA", .execute = txa },
  [INST_TYA] = { .mnemonic = "TYA", .execute = tya },

  /* Stack Operations */
  [INST_TSX] = { .mnemonic = "TSX", .execute = tsx },
  [INST_TXS] = { .mnemonic = "TXS", .execute = txs },
  [INST_PHA] = { .mnemonic = "PHA", .execute = pha },
  [INST_PHP] = { .mnemonic = "PHP", .execute = php },
  [INST_PLA] = { .mnemonic = "PLA", .execute = pla },
  [INST_PLP] = { .mnemonic = "PLP", .execute = plp },

  /* Logical */
  [INST_AND] = { .mnemonic = "AND", .execute = and },
  [INST_EOR] = { .mnemonic = "EOR", .execute = eor },
  [INST_ORA] = { .mnemonic = "ORA", .execute = ora },
  [INST_BIT] = { .mnemonic = "BIT", .execute = bit },

  /* Arithmetic */
  [INST_ADC] = { .mnemonic = "ADC", .execute = adc },
  [INST_SBC] = { .mnemonic = "SBC", .execute = sbc },
  [INST_CMP] = { .mnemonic = "CMP", .execute = cmp },
  [INST_CPX] = { .mnemonic = "CPX", .execute = cpx },
  [INST_CPY] = { .mnemonic = "CPY", .execute = cpy },

  /* Increments & Decrements */
  [INST_INC] = { .mnemonic = "INC", .execute = inc },
  [INST_INX] = { .mnemonic = "INX", .execute = inx },
  [INST_INY] = { .mnemonic = "INY", .execute = iny },
  [INST_DEC] = { .mnemonic = "DEC", .execute = dec },
  [INST_DEX] = { .mnemonic = "DEX", .execute = dex },
  [INST_DEY] = { .mnemonic = "DEY", .execute = dey },

  /* Shifts */
  [INST_ASL] = { .mnemonic = "ASL", .execute = asl },
  [INST_LSR] = { .mnemonic = "LSR", .execute = lsr },
  [INST_ROL] = { .mnemonic = "ROL", .execute = rol },
  [INST_ROR] = { .mnemonic = "ROR", .execute = ror },

  /* Jumps & Calls */
  [INST_JMP] = { .mnemonic = "JMP", .execute = jmp },
  [INST_JSR] = { .mnemonic = "JSR", .execute = jsr },
  [INST_RTS] = { .mnemonic = "RTS", .execute = rts },

  /* Branches */
  [INST_BCC] = { .mnemonic = "BCC", .execute = bcc },
  [INST_BCS] = { .mnemonic = "BCS", .execute = bcs },
  [INST_BEQ] = { .mnemonic = "BEQ", .execute = beq },
  [INST_BMI] = { .mnemonic = "BMI", .execute = bmi },
  [INST_BNE] = { .mnemonic = "BNE", .execute = bne },
  [INST_BPL] = { .mnemonic = "BPL", .execute = bpl },
  [INST_BVC] = { .mnemonic = "BVC", .execute = bvc },
  [INST_BVS] = { .mnemonic = "BVS", .execute = bvs },

  /* Status Flag Changes */
  [INST_CLC] = { .mnemonic = "CLC", .execute = clc },
  [INST_CLD] = { .mnemonic = "CLD", .execute = cld },
  [INST_CLI] = { .mnemonic = "CLI", .execute = cli },
  [INST_CLV] = { .mnemonic = "CLV", .execute = clv },
  [INST_SEC] = { .mnemonic = "SEC", .execute = sec },
  [INST_SED] = { .mnemonic = "SED", .execute = sed },
  [INST_SEI] = { .mnemonic = "SEI", .execute = sei },

  /* System Functions */
  [INST_BRK] = { .mnemonic = "BRK", .execute = brk },
  [INST_NOP] = { .mnemonic = "NOP", .execute = nop },
  [INST_RTI] = { .mnemonic = "RTI", .execute = rti },

  /* Undocumented Instructions */
  [INST_KIL] = { .mnemonic = "*KIL", .execute = kil },
  [INST_SLO] = { .mnemonic = "*SLO", .execute = slo },
  [INST_ANC] = { .mnemonic = "*ANC", .execute = anc },
  [INST_RLA] = { .mnemonic = "*RLA", .execute = rla },
  [INST_SRE] = { .mnemonic = "*SRE", .execute = sre },
  [INST_ALR] = { .mnemonic = "*ALR", .execute = alr },
  [INST_RRA] = { .mnemonic = "*RRA", .execute = rra },
  [INST_ARR] = { .mnemonic = "*ARR", .execute = arr },
  [INST_SAX] = { .mnemonic = "*SAX", .execute = sax },
  [INST_XAA] = { .mnemonic = "*XAA", .execute = xaa },
  [INST_AHX] = { .mnemonic = "*AHX", .execute = ahx },
};