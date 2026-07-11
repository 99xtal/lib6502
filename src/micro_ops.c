#include "micro_ops.h"

#include <string.h>

#include "6502.h"
#include "flags.h"
#include "stack.h"

uint8_t read(CPU6502* cpu, uint16_t addr) { return cpu->read(cpu->ctx, addr); }

void write(CPU6502* cpu, uint16_t addr, uint8_t value) {
  cpu->write(cpu->ctx, addr, value);
}

uint16_t full_addr(CPU6502* cpu) {
  return (cpu->op.addr_hi << 8) | cpu->op.addr_lo;
}

void finish_op(CPU6502* cpu) { memset(&cpu->op, 0, sizeof(cpu->op)); }

void dummy_read(CPU6502* cpu, uint16_t addr) { read(cpu, addr); }

void inc_sp(CPU6502* cpu) { cpu->SP++; }

void dummy_pc_read(CPU6502* cpu) { read(cpu, cpu->PC); }

void dummy_pc_read_and_inc(CPU6502* cpu) { read(cpu, cpu->PC++); }

void dummy_stack_read(CPU6502* cpu) { read(cpu, 0x0100 | cpu->SP); }

void dummy_temp_addr_read(CPU6502* cpu) { read(cpu, cpu->op.temp_addr); }

void read_pc_addr_low(CPU6502* cpu) {
  cpu->op.addr_lo = read(cpu, cpu->PC++);
  cpu->op.addr = cpu->op.addr_lo;
}

void read_pc_addr_high(CPU6502* cpu) { cpu->op.addr_hi = read(cpu, cpu->PC++); }

void read_addr_data(CPU6502* cpu) { cpu->op.data = read(cpu, full_addr(cpu)); }

void fetch_ptr(CPU6502* cpu) { cpu->op.ptr = read(cpu, cpu->PC++); }

void read_ptr_addr_low(CPU6502* cpu) {
  cpu->op.addr_lo = read(cpu, cpu->op.ptr);
}

void read_ptr_addr_high(CPU6502* cpu) {
  cpu->op.addr_hi = read(cpu, (uint8_t)(cpu->op.ptr + 1));
  cpu->op.addr = full_addr(cpu);
}

void read_ptr_add_x(CPU6502* cpu) {
  read(cpu, cpu->op.ptr);  // dummy read
  cpu->op.ptr = (uint8_t)(cpu->op.ptr + cpu->X);
}

void read_ptr_addr_high_add_y(CPU6502* cpu) {
  cpu->op.addr_hi = read(cpu, (uint8_t)(cpu->op.ptr + 1));

  uint16_t base = full_addr(cpu);
  uint16_t addr = base + cpu->Y;

  cpu->op.addr = addr;
  cpu->op.addr_lo = addr & 0xFF;
  cpu->op.addr_hi = (addr & 0xFF00) >> 8;
  cpu->op.page_crossed = (base & 0xFF00) != (addr & 0xFF00);

  cpu->op.temp_addr =
      ((uint16_t)cpu->op.addr_hi << 8) | ((cpu->op.addr_lo + cpu->Y) & 0xFF);
}

void read_pc_addr_low_add_x(CPU6502* cpu) {
  dummy_read(cpu, cpu->op.addr_lo);
  cpu->op.addr_lo += cpu->X;
  cpu->op.addr = cpu->op.addr_lo;
}

void read_pc_addr_low_add_y(CPU6502* cpu) {
  dummy_read(cpu, cpu->op.addr_lo);
  cpu->op.addr_lo += cpu->Y;
}

void read_pc_addr_high_add_x(CPU6502* cpu) {
  read_pc_addr_high(cpu);

  uint16_t base = full_addr(cpu);
  uint16_t addr = base + cpu->X;
  cpu->op.addr_hi = (addr & 0xFFEE) >> 8;
  cpu->op.addr_lo = (addr & 0xFF);

  cpu->op.addr = addr;
  cpu->op.page_crossed = (base & 0xFF00) != (addr & 0xFF00);

  cpu->op.temp_addr =
      ((uint16_t)cpu->op.addr_hi << 8) | ((cpu->op.addr_lo + cpu->X) & 0xFF);
}

void read_pc_addr_high_add_y(CPU6502* cpu) {
  read_pc_addr_high(cpu);

  uint16_t base = full_addr(cpu);
  uint16_t addr = base + cpu->Y;

  cpu->op.addr = addr;
  cpu->op.page_crossed = (base & 0xFF00) != (addr & 0xFF00);

  cpu->op.temp_addr =
      ((uint16_t)cpu->op.addr_hi << 8) | ((cpu->op.addr_lo + cpu->X) & 0xFF);
}

void fetch_pointer_addr_low(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  cpu->op.ptr = addr;
  cpu->op.addr_lo = read(cpu, addr);
}

void push_pc_high(CPU6502* cpu) { stack_push_u8(cpu, (cpu->PC >> 8) & 0xFF); }

void push_pc_low(CPU6502* cpu) { stack_push_u8(cpu, cpu->PC & 0xFF); }

void pull_pc_low(CPU6502* cpu) {
  cpu->op.addr_lo = cpu->read(cpu->ctx, 0x0100 | cpu->SP);
  cpu->SP++;
}

void pull_pc_high_no_inc(CPU6502* cpu) {
  cpu->op.addr_hi = read(cpu, 0x0100 | cpu->SP);
}

void pull_pc_high_finish(CPU6502* cpu) {
  cpu->op.addr_hi = cpu->read(cpu->ctx, 0x0100 | cpu->SP);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;

  cpu->status &= ~FLAG_BREAK;
  cpu->status |= FLAG_UNUSED;

  finish_op(cpu);
}

void push_p_with_break(CPU6502* cpu) {
  uint8_t status = cpu->status;
  status |= FLAG_BREAK;
  status |= FLAG_UNUSED;  // bit 5 is usually always pushed as 1

  stack_push_u8(cpu, status);
}

void push_p_without_break(CPU6502* cpu) {
  uint8_t status = cpu->status;
  status &= ~FLAG_BREAK;
  status |= FLAG_UNUSED;  // bit 5 is usually always pushed as 1

  stack_push_u8(cpu, status);
}

void pull_p(CPU6502* cpu) {
  cpu->status = cpu->read(cpu->ctx, 0x0100 | cpu->SP);
  cpu->SP++;
}

void write_data_and_finish(CPU6502* cpu) {
  write(cpu, full_addr(cpu), cpu->op.data);
  finish_op(cpu);
}

void read_irq_low(CPU6502* cpu) {
  cpu->op.addr_lo = cpu->read(cpu->ctx, 0xFFFE);
}

void read_irq_high_finish(CPU6502* cpu) {
  cpu->op.addr_hi = cpu->read(cpu->ctx, 0xFFFF);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;
  cpu->status |= FLAG_INTERRUPT_DISABLE;

  finish_op(cpu);
}

void read_nmi_low(CPU6502* cpu) {
  cpu->op.addr_lo = cpu->read(cpu->ctx, 0xFFFA);
}

void read_nmi_high_finish(CPU6502* cpu) {
  cpu->op.addr_hi = cpu->read(cpu->ctx, 0xFFFB);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;
  cpu->status |= FLAG_INTERRUPT_DISABLE;

  finish_op(cpu);
}

void set_nz(CPU6502* cpu, uint8_t value) {
  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_ZERO, value == 0 ? 1 : 0);
}

/**
 * Load/Store Operations
 */
void ldx_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);
  cpu->X = value;
  set_nz(cpu, value);
  finish_op(cpu);
}

void ldx_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);

  cpu->X = value;
  set_nz(cpu, value);
  finish_op(cpu);
}

void ldy_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);

  cpu->Y = value;
  set_nz(cpu, value);
  finish_op(cpu);
}

void ldx_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    cpu->X = value;
    set_nz(cpu, cpu->X);
    finish_op(cpu);
  }
}

void ldx_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  cpu->X = value;
  set_nz(cpu, cpu->X);
  finish_op(cpu);
}

void ldy_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);
  cpu->Y = value;
  set_nz(cpu, value);
  finish_op(cpu);
}

void ldy_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    cpu->Y = value;
    set_nz(cpu, cpu->Y);
    finish_op(cpu);
  }
}

void ldy_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  cpu->Y = value;
  set_nz(cpu, cpu->Y);
  finish_op(cpu);
}

void lda_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);

  cpu->A = value;
  set_nz(cpu, value);

  finish_op(cpu);
}

void lda_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);

  cpu->A = value;
  set_nz(cpu, value);

  finish_op(cpu);
}

void lda_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    cpu->A = value;
    set_nz(cpu, cpu->A);
    finish_op(cpu);
  }
}

void lda_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  cpu->A = value;
  set_nz(cpu, cpu->A);
  finish_op(cpu);
}

void sta_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  dummy_read(cpu, addr);

  if (!cpu->op.page_crossed) {
    cpu->write(cpu->ctx, addr, cpu->A);

    finish_op(cpu);
  }
}

void sta_indexed_rewrite_fixed(CPU6502* cpu) {
  cpu->write(cpu->ctx, cpu->op.addr, cpu->A);

  finish_op(cpu);
}

void sta_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  cpu->write(cpu->ctx, addr, cpu->A);

  finish_op(cpu);
}

void stx_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  cpu->write(cpu->ctx, addr, cpu->X);

  finish_op(cpu);
}

void sty_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  cpu->write(cpu->ctx, addr, cpu->Y);

  finish_op(cpu);
}

/**
 * Stack Operations
 */
void txs_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  cpu->SP = cpu->X;
  finish_op(cpu);
}

void tsx_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);

  cpu->X = cpu->SP;
  set_nz(cpu, cpu->X);

  finish_op(cpu);
}

void tax_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);

  cpu->X = cpu->A;
  set_nz(cpu, cpu->X);

  finish_op(cpu);
}

void txa_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);

  cpu->A = cpu->X;
  set_nz(cpu, cpu->A);

  finish_op(cpu);
}

void tya_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);

  cpu->A = cpu->Y;
  set_nz(cpu, cpu->A);

  finish_op(cpu);
}

void tay_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);

  cpu->Y = cpu->A;
  set_nz(cpu, cpu->Y);

  finish_op(cpu);
}

void pha_imp(CPU6502* cpu) {
  stack_push_u8(cpu, cpu->A);

  finish_op(cpu);
}

void php_imp(CPU6502* cpu) {
  uint8_t status = cpu->status;
  status |= FLAG_BREAK;
  status |= FLAG_UNUSED;  // bit 5 is usually always pushed as 1

  stack_push_u8(cpu, status);

  finish_op(cpu);
}

void pla_imp(CPU6502* cpu) {
  cpu->A = cpu->read(cpu->ctx, 0x0100 | cpu->SP);

  set_nz(cpu, cpu->A);

  finish_op(cpu);
}

void plp_imp(CPU6502* cpu) {
  uint8_t status_value = cpu->read(cpu->ctx, 0x0100 | cpu->SP);

  cpu->status = (status_value & ~FLAG_BREAK) | FLAG_UNUSED;

  finish_op(cpu);
}

/**
 * Logical
 */
void alu_and(CPU6502* cpu, uint8_t value) {
  cpu->A &= value;
  set_nz(cpu, cpu->A);
}

void and_imm(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);

  alu_and(cpu, value);

  finish_op(cpu);
}

void and_addr(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, full_addr(cpu));

  alu_and(cpu, value);

  finish_op(cpu);
}

void and_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    alu_and(cpu, value);
    finish_op(cpu);
  }
}

void and_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  alu_and(cpu, value);

  finish_op(cpu);
}

void alu_eor(CPU6502* cpu, uint8_t value) {
  cpu->A ^= value;

  set_nz(cpu, cpu->A);
}

void eor_imm(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);

  alu_eor(cpu, value);

  finish_op(cpu);
}

void eor_addr(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, full_addr(cpu));

  alu_eor(cpu, value);

  finish_op(cpu);
}

void eor_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    alu_eor(cpu, value);
    finish_op(cpu);
  }
}

void eor_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  alu_eor(cpu, value);

  finish_op(cpu);
}

void alu_ora(CPU6502* cpu, uint8_t value) {
  cpu->A |= value;

  set_nz(cpu, cpu->A);
}

void ora_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);

  alu_ora(cpu, value);

  finish_op(cpu);
}

void ora_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);

  alu_ora(cpu, value);

  finish_op(cpu);
}

void ora_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    alu_ora(cpu, value);
    finish_op(cpu);
  }
}

void ora_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  alu_ora(cpu, value);

  finish_op(cpu);
}

void bit_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = cpu->read(cpu->ctx, addr);

  set_flag(cpu, FLAG_ZERO, (cpu->A & value) == 0);
  set_flag(cpu, FLAG_NEGATIVE, (value & 0x80) != 0);
  set_flag(cpu, FLAG_OVERFLOW, (value & 0x40) != 0);

  finish_op(cpu);
}

/**
 * Arithmetic
 */
void cmp_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);
  uint8_t result = (uint16_t)cpu->A - value;

  set_flag(cpu, FLAG_CARRY, cpu->A >= value);
  set_flag(cpu, FLAG_ZERO, cpu->A == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void cmp_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);
  uint8_t result = (uint16_t)cpu->A - value;

  set_flag(cpu, FLAG_CARRY, cpu->A >= value);
  set_flag(cpu, FLAG_ZERO, cpu->A == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void cmp_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    uint8_t result = (uint16_t)cpu->A - value;

    set_flag(cpu, FLAG_CARRY, cpu->A >= value);
    set_flag(cpu, FLAG_ZERO, cpu->A == value);
    set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

    finish_op(cpu);
  }
}

void cmp_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  uint8_t result = (uint16_t)cpu->A - value;

  set_flag(cpu, FLAG_CARRY, cpu->A >= value);
  set_flag(cpu, FLAG_ZERO, cpu->A == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void cpy_imm(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t result = (uint16_t)cpu->Y - value;

  set_flag(cpu, FLAG_CARRY, cpu->Y >= value);
  set_flag(cpu, FLAG_ZERO, cpu->Y == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void cpy_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = cpu->read(cpu->ctx, addr);
  uint8_t result = (uint16_t)cpu->Y - value;

  set_flag(cpu, FLAG_CARRY, cpu->Y >= value);
  set_flag(cpu, FLAG_ZERO, cpu->Y == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void alu_adc(CPU6502* cpu, uint8_t value) {
  uint8_t carry_in = get_flag(cpu, FLAG_CARRY);

  if (get_flag(cpu, FLAG_DECIMAL_MODE) &&
      cpu->variant != CPU6502_VARIANT_RP2A03) {
    uint16_t low_nibble = (cpu->A & 0x0F) + (value & 0x0F) + carry_in;
    if (low_nibble > 0x09) {
      low_nibble += 0x06;
    }

    uint16_t carry_to_high = (low_nibble >> 4) & 0x01;
    uint16_t high_nibble =
        (cpu->A & 0xF0) + (value & 0xF0) + (carry_to_high << 4);

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
             (~(cpu->A ^ value) & (cpu->A ^ binary_result) & 0x80) != 0);
    cpu->A = binary_result;
  }
}

void adc_imm(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);

  alu_adc(cpu, value);

  finish_op(cpu);
}

void adc_addr(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, full_addr(cpu));

  alu_adc(cpu, value);

  finish_op(cpu);
}

void adc_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    alu_adc(cpu, value);

    finish_op(cpu);
  }
}

void adc_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  alu_adc(cpu, value);

  finish_op(cpu);
}

void alu_sbc(CPU6502* cpu, uint8_t value) {
  uint8_t carry = get_flag(cpu, FLAG_CARRY) ? 1 : 0;
  uint8_t old_a = cpu->A;

  uint16_t binary_diff = (uint16_t)old_a + (uint8_t)(~value) + carry;
  uint8_t binary_result = (uint8_t)binary_diff;

  set_flag(cpu, FLAG_CARRY, binary_diff > 0xFF);
  set_flag(cpu, FLAG_ZERO, binary_result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (binary_result & 0x80) != 0);
  set_flag(cpu, FLAG_OVERFLOW,
           ((old_a ^ value) & (old_a ^ binary_result) & 0x80) != 0);

  if (get_flag(cpu, FLAG_DECIMAL_MODE) &&
      cpu->variant != CPU6502_VARIANT_RP2A03) {
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

void sbc_imm(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);

  alu_sbc(cpu, value);

  finish_op(cpu);
}

void sbc_addr(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, full_addr(cpu));

  alu_sbc(cpu, value);

  finish_op(cpu);
}

void sbc_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    alu_adc(cpu, value);

    finish_op(cpu);
  }
}

void sbc_indexed_reread_fixed(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->op.addr);

  alu_sbc(cpu, value);

  finish_op(cpu);
}

void cpx_imm(CPU6502* cpu) {
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);
  uint8_t result = (uint16_t)cpu->X - value;

  set_flag(cpu, FLAG_CARRY, cpu->X >= value);
  set_flag(cpu, FLAG_ZERO, cpu->X == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void cpx_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = cpu->read(cpu->ctx, addr);
  uint8_t result = (uint16_t)cpu->X - value;

  set_flag(cpu, FLAG_CARRY, cpu->X >= value);
  set_flag(cpu, FLAG_ZERO, cpu->X == value);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

/**
 * Increments & Decrements
 */
void inx_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  cpu->X++;
  set_nz(cpu, cpu->X);
  finish_op(cpu);
}

void iny_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  cpu->Y++;
  set_nz(cpu, cpu->Y);
  finish_op(cpu);
}

void dex_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  cpu->X--;
  set_nz(cpu, cpu->X);
  finish_op(cpu);
}

void dey_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  cpu->Y--;
  set_nz(cpu, cpu->Y);
  finish_op(cpu);
}

void inc_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);

  cpu->op.data++;

  set_nz(cpu, cpu->op.data);
}

void dec_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);

  cpu->op.data--;

  set_nz(cpu, cpu->op.data);
}

/**
 * Shifts
 */
void asl_acc(CPU6502* cpu) {
  uint8_t value = cpu->A;

  uint8_t result = value << 1;
  uint8_t last_bit = (value & 0x80) != 0;

  cpu->A = result;

  set_flag(cpu, FLAG_CARRY, last_bit);
  set_nz(cpu, cpu->A);

  finish_op(cpu);
}

void asl_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);  // old value

  uint8_t result = cpu->op.data << 1;
  uint8_t last_bit = (cpu->op.data & 0x80) != 0;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, last_bit);
  set_nz(cpu, cpu->op.data);
}

void lsr_acc(CPU6502* cpu) {
  uint8_t value = cpu->A;

  uint8_t result = value >> 1;
  uint8_t first_bit = (value & 0x01) != 0;

  cpu->A = result;

  set_flag(cpu, FLAG_CARRY, first_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, 0);

  finish_op(cpu);
}

void lsr_dummy_write_and_compute(CPU6502* cpu) {
  uint8_t result = cpu->op.data >> 1;
  uint8_t first_bit = (cpu->op.data & 0x01) != 0;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, first_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, 0);
}

void rol_acc(CPU6502* cpu) {
  uint8_t value = cpu->A;

  uint8_t result = value << 1;
  uint8_t old_last_bit = (value & 0x80) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit;

  cpu->A = result;

  set_flag(cpu, FLAG_CARRY, old_last_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void rol_dummy_write_and_compute(CPU6502* cpu) {
  uint8_t value = cpu->op.data;

  uint8_t result = value << 1;
  uint8_t old_last_bit = (value & 0x80) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, old_last_bit);
  set_nz(cpu, result);
}

void ror_acc(CPU6502* cpu) {
  uint8_t value = cpu->A;

  uint8_t result = value >> 1;
  uint8_t old_first_bit = (value & 0x01) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit << 7;

  cpu->A = result;

  set_flag(cpu, FLAG_CARRY, old_first_bit);
  set_nz(cpu, result);

  finish_op(cpu);
}

void ror_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);  // old value

  uint8_t result = cpu->op.data >> 1;
  uint8_t old_first_bit = (cpu->op.data & 0x01) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit << 7;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, old_first_bit);
  set_nz(cpu, result);
}

/**
 * Jumps & Calls
 */
void jmp_abs_finish(CPU6502* cpu) {
  cpu->op.addr_hi = read(cpu, cpu->PC);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;

  finish_op(cpu);
}

void jsr_abs(CPU6502* cpu) {
  cpu->op.addr_hi = read(cpu, cpu->PC);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;

  finish_op(cpu);
}

void jmp_ind(CPU6502* cpu) {
  /**
   * Original 6502 JMP ($xxxx) bug.
   *
   * The CPU incorrectly wraps the high-byte fetch within the same page.
   *
   * Example:
   *   JMP ($30FF)
   *   low byte  = memory[$30FF]
   *   high byte = memory[$3000]  // should be $3100
   */
  uint16_t target_high_addr =
      (cpu->op.ptr & 0xFF00) | ((cpu->op.ptr + 1) & 0x00FF);

  cpu->op.addr_hi = cpu->read(cpu->ctx, target_high_addr);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;

  finish_op(cpu);
}

void rts_finish(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr + 1;

  finish_op(cpu);
}

/**
 * Branches
 */
void bne_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (get_flag(cpu, FLAG_ZERO)) {
    finish_op(cpu);
  }
}

void beq_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (!get_flag(cpu, FLAG_ZERO)) {
    finish_op(cpu);
  }
}

void bpl_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (get_flag(cpu, FLAG_NEGATIVE)) {
    finish_op(cpu);
  }
}

void bmi_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (!get_flag(cpu, FLAG_NEGATIVE)) {
    finish_op(cpu);
  }
}

void bvc_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (get_flag(cpu, FLAG_OVERFLOW)) {
    finish_op(cpu);
  }
}

void bvs_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (!get_flag(cpu, FLAG_OVERFLOW)) {
    finish_op(cpu);
  }
}

void bcc_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (get_flag(cpu, FLAG_CARRY)) {
    finish_op(cpu);
  }
}

void bcs_fetch_offset(CPU6502* cpu) {
  cpu->op.offset = (int8_t)read(cpu, cpu->PC++);

  if (!get_flag(cpu, FLAG_CARRY)) {
    finish_op(cpu);
  }
}

void branch_op(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);

  uint16_t old_pc = cpu->PC;
  cpu->PC += cpu->op.offset;

  if ((old_pc & 0xFF00) == (cpu->PC & 0xFF00)) {
    finish_op(cpu);  // same page
  }

  cpu->op.old_pc = old_pc;
}

void branch_page_fix(CPU6502* cpu) {
  read(cpu, (cpu->PC & 0x00FF) | (cpu->op.old_pc & 0xFF00));

  finish_op(cpu);
}

/**
 * Status Flag Instructions
 */
void cld_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_DECIMAL_MODE, 0);
  finish_op(cpu);
}

void clc_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_CARRY, 0);
  finish_op(cpu);
}

void cli_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 0);
  finish_op(cpu);
}

void clv_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_OVERFLOW, 0);
  finish_op(cpu);
}

void sec_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_CARRY, 1);
  finish_op(cpu);
}

void sed_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_DECIMAL_MODE, 1);
  finish_op(cpu);
}

void sei_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);
  finish_op(cpu);
}

/**
 * System functions
 */

void nop_imp(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC);
  finish_op(cpu);
}

void nop_imm(CPU6502* cpu) {
  dummy_read(cpu, cpu->PC++);
  finish_op(cpu);
}

void nop_addr(CPU6502* cpu) {
  dummy_read(cpu, full_addr(cpu));
  finish_op(cpu);
}

void nop_indexed_read_maybe_finish(CPU6502* cpu) {
  uint16_t addr = cpu->op.page_crossed ? cpu->op.temp_addr : cpu->op.addr;
  uint8_t value = read(cpu, addr);

  if (!cpu->op.page_crossed) {
    finish_op(cpu);
  }
}

/**
 * Undocumented Opcodes
 */

void kil_imp(CPU6502* cpu) { cpu->jammed = true; }

void anc_imm(CPU6502* cpu) {
  // AND
  uint8_t value = cpu->read(cpu->ctx, cpu->PC++);

  alu_and(cpu, value);

  // set carry as if ASL/ROL was performed
  uint8_t last_bit = (cpu->A & 0x80) != 0;
  set_flag(cpu, FLAG_CARRY, last_bit);

  finish_op(cpu);
}

void alr_imm(CPU6502* cpu) {
  // AND
  uint8_t value = read(cpu, cpu->PC++);
  alu_and(cpu, value);

  // LSR
  uint8_t result = cpu->A >> 1;
  set_flag(cpu, FLAG_CARRY, cpu->A & 0x01);

  cpu->A = result;

  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, 0);

  finish_op(cpu);
}

void arr_imm(CPU6502* cpu) {
  // AND
  uint8_t value = read(cpu, cpu->PC++);

  alu_and(cpu, value);

  uint8_t carry_in = get_flag(cpu, FLAG_CARRY);

  uint8_t result = (cpu->A >> 1) | (carry_in << 7);
  cpu->A = result;

  set_flag(cpu, FLAG_CARRY, cpu->A & 0x40);
  set_flag(cpu, FLAG_OVERFLOW, ((cpu->A >> 6) ^ (cpu->A >> 5)) & 1);
  set_nz(cpu, cpu->A);

  finish_op(cpu);
}

void xaa_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);

  cpu->A = cpu->X;
  set_nz(cpu, cpu->A);

  cpu->A &= value;

  finish_op(cpu);
}

void lax_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);

  cpu->A = value;
  cpu->X = value;

  finish_op(cpu);
}

void lax_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);

  cpu->A = value;
  cpu->X = value;
  set_nz(cpu, value);

  finish_op(cpu);
}

void axs_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);
  uint8_t source = cpu->A & cpu->X;
  uint8_t result = source - value;

  cpu->X = result;

  set_flag(cpu, FLAG_CARRY, source >= value);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);

  finish_op(cpu);
}

void slo_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);  // old value

  // ASL
  uint8_t result = cpu->op.data << 1;
  uint8_t last_bit = (cpu->op.data & 0x80) != 0;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, last_bit);
  set_nz(cpu, cpu->op.data);

  // ORA
  alu_ora(cpu, result);
}

void sre_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);  // old value

  // LSR
  uint8_t result = cpu->op.data >> 1;
  uint8_t first_bit = (cpu->op.data & 0x01) != 0;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, first_bit);
  set_flag(cpu, FLAG_ZERO, result == 0);
  set_flag(cpu, FLAG_NEGATIVE, 0);

  // EOR
  alu_eor(cpu, result);
}

void rla_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);  // old value

  // ROL
  uint8_t value = cpu->op.data;
  uint8_t result = value << 1;
  uint8_t old_last_bit = (value & 0x80) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, old_last_bit);

  // AND
  alu_and(cpu, result);
}

void rra_dummy_write_and_compute(CPU6502* cpu) {
  // ROR
  write(cpu, cpu->op.addr, cpu->op.data);  // old value

  uint8_t result = cpu->op.data >> 1;
  uint8_t old_first_bit = (cpu->op.data & 0x01) != 0;
  uint8_t carry_bit = get_flag(cpu, FLAG_CARRY);

  result |= carry_bit << 7;

  cpu->op.data = result;

  set_flag(cpu, FLAG_CARRY, old_first_bit);
  set_nz(cpu, result);

  // ADC
  alu_adc(cpu, result);
}

void sax_addr(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  cpu->write(cpu->ctx, addr, cpu->A & cpu->X);

  finish_op(cpu);
}

void dcp_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);

  cpu->op.data--;

  uint8_t result = (uint16_t)cpu->A - cpu->op.data;

  set_flag(cpu, FLAG_CARRY, cpu->A >= cpu->op.data);
  set_flag(cpu, FLAG_ZERO, cpu->A == cpu->op.data);
  set_flag(cpu, FLAG_NEGATIVE, (result & 0x80) != 0);
}

void isc_dummy_write_and_compute(CPU6502* cpu) {
  write(cpu, cpu->op.addr, cpu->op.data);

  cpu->op.data++;

  alu_sbc(cpu, cpu->op.data);
}

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

const OpDef reset_sequence = {
    .name = "RESET",
    .micro_ops =
        {
            dummy,
            dummy,
            dec_sp,
            dec_sp,
            dec_sp,
            read_reset_low,
            read_reset_high_finish,
        },
};

const OpDef nmi_sequence = {
    .name = "NMI",
    .micro_ops =
        {
            dummy_pc_read,
            push_pc_high,
            push_pc_low,
            push_p_without_break,
            read_nmi_low,
            read_nmi_high_finish,
        },
};

const OpDef
    instruction_defs[256] =
        {
            [0x00] =
                {
                    .name = "BRK",
                    .micro_ops =
                        {
                            dummy_pc_read_and_inc,
                            push_pc_high,
                            push_pc_low,
                            push_p_with_break,
                            read_irq_low,
                            read_irq_high_finish,
                        },
                },
            [0x01] =
                {
                    .name = "ORA indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            ora_addr,
                        },
                },
            [0x02] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x03] =
                {
                    .name = "*SLO indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            read_addr_data,
                            slo_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x04] =
                {
                    .name = "*NOP zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            nop_addr,
                        },
                },
            [0x05] =
                {
                    .name = "ORA zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            ora_addr,
                        },
                },
            [0x06] =
                {
                    .name = "ASL zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            asl_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x07] =
                {
                    .name = "*SLO zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            slo_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x08] =
                {
                    .name = "PHP",
                    .micro_ops =
                        {
                            dummy_pc_read,
                            php_imp,
                        },
                },
            [0x09] =
                {
                    .name = "ORA imm.",
                    .micro_ops =
                        {
                            ora_imm,
                        },
                },
            [0x0A] =
                {
                    .name = "ASL acc.",
                    .micro_ops =
                        {
                            asl_acc,
                        },
                },
            [0x0B] =
                {
                    .name = "*ANC imm.",
                    .micro_ops =
                        {
                            anc_imm,
                        },
                },
            [0x0C] =
                {
                    .name = "*NOP abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            nop_addr,
                        },
                },
            [0x0D] =
                {
                    .name = "ORA abs.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            ora_addr,
                        },
                },
            [0x0E] =
                {
                    .name = "ASL abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            asl_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x0F] =
                {
                    .name = "*SLO abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            slo_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x10] =
                {
                    .name = "BPL",
                    .micro_ops =
                        {
                            bpl_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0x11] =
                {
                    .name = "ORA ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            ora_indexed_read_maybe_finish,
                            ora_indexed_reread_fixed,
                        },
                },
            [0x12] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x14] =
                {
                    .name = "*NOP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            nop_addr,
                        },
                },
            [0x15] =
                {
                    .name = "ORA zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            ora_addr,
                        },
                },
            [0x16] =
                {
                    .name = "ASL zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            asl_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x17] =
                {
                    .name = "*SLO zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            slo_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x18] =
                {
                    .name = "CLC",
                    .micro_ops =
                        {
                            clc_imp,
                        },
                },
            [0x19] =
                {
                    .name = "ORA abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            ora_indexed_read_maybe_finish,
                            ora_indexed_reread_fixed,
                        },
                },
            [0x1A] =
                {
                    .name = "*NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0x1C] =
                {
                    .name = "*NOP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            nop_indexed_read_maybe_finish,
                            nop_addr,
                        },
                },
            [0x1D] =
                {
                    .name = "ORA abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            ora_indexed_read_maybe_finish,
                            ora_indexed_reread_fixed,
                        },
                },
            [0x1E] =
                {
                    .name = "ASL abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            dummy_temp_addr_read,
                            read_addr_data,
                            asl_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x20] =
                {
                    .name = "JSR abs.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            dummy_stack_read,
                            push_pc_high,
                            push_pc_low,
                            jsr_abs,
                        },
                },
            [0x21] =
                {
                    .name = "AND indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            and_addr,
                        },
                },
            [0x22] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x23] =
                {
                    .name = "*RLA indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            read_addr_data,
                            rla_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x24] =
                {
                    .name = "BIT zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            bit_addr,
                        },
                },
            [0x25] =
                {
                    .name = "AND zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            and_addr,
                        },
                },
            [0x26] =
                {
                    .name = "ROL zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            rol_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x27] =
                {
                    .name = "*RLA zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            rla_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x28] =
                {
                    .name = "PLP",
                    .micro_ops =
                        {
                            dummy_pc_read,
                            inc_sp,
                            plp_imp,
                        },
                },
            [0x29] =
                {
                    .name = "AND imm",
                    .micro_ops =
                        {
                            and_imm,
                        },
                },
            [0x2A] =
                {
                    .name = "ROL acc.",
                    .micro_ops =
                        {
                            rol_acc,
                        },
                },
            [0x2B] =
                {
                    .name = "*ANC imm.",
                    .micro_ops =
                        {
                            anc_imm,
                        },
                },
            [0x2C] =
                {
                    .name = "BIT abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            bit_addr,
                        },
                },
            [0x2D] =
                {
                    .name = "AND abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            and_addr,
                        },
                },
            [0x2E] =
                {
                    .name = "ROL abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            rol_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x2F] =
                {
                    .name = "*RLA abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            rla_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x30] =
                {
                    .name = "BMI",
                    .micro_ops =
                        {
                            bmi_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0x31] =
                {
                    .name = "AND ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            and_indexed_read_maybe_finish,
                            and_indexed_reread_fixed,
                        },
                },
            [0x32] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x34] =
                {
                    .name = "*NOP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            nop_addr,
                        },
                },
            [0x35] =
                {
                    .name = "AND zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            and_addr,
                        },
                },
            [0x36] =
                {
                    .name = "ROL zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            rol_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x37] =
                {
                    .name = "*RLA zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,

                            read_addr_data,
                            rla_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x38] =
                {
                    .name = "SEC",
                    .micro_ops =
                        {
                            sec_imp,
                        },
                },
            [0x39] =
                {
                    .name = "AND abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            and_indexed_read_maybe_finish,
                            and_indexed_reread_fixed,
                        },
                },
            [0x3A] =
                {
                    .name = "*NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0x3C] =
                {
                    .name = "*NOP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            nop_indexed_read_maybe_finish,
                            nop_addr,
                        },
                },
            [0x3D] =
                {
                    .name = "AND abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            and_indexed_read_maybe_finish,
                            and_indexed_reread_fixed,
                        },
                },
            [0x3E] =
                {
                    .name = "ROL abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            dummy_temp_addr_read,
                            read_addr_data,
                            rol_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x40] =
                {
                    .name = "RTI",
                    .micro_ops =
                        {
                            dummy_pc_read_and_inc,
                            inc_sp,
                            pull_p,
                            pull_pc_low,
                            pull_pc_high_finish,
                        },
                },
            [0x41] =
                {
                    .name = "EOR indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            eor_addr,
                        },
                },
            [0x42] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x43] =
                {
                    .name = "*SRE indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            read_addr_data,
                            sre_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x44] =
                {
                    .name = "*NOP zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            nop_addr,
                        },
                },
            [0x45] =
                {
                    .name = "EOR zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            eor_addr,
                        },
                },
            [0x46] =
                {
                    .name = "LSR zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            lsr_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x47] =
                {
                    .name = "*SRE zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            sre_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x48] =
                {
                    .name = "PHA",
                    .micro_ops =
                        {
                            dummy_pc_read,
                            pha_imp,
                        },
                },
            [0x49] =
                {
                    .name = "EOR imm.",
                    .micro_ops =
                        {
                            eor_imm,
                        },
                },
            [0x4A] =
                {
                    .name = "LSR acc.",
                    .micro_ops =
                        {
                            lsr_acc,
                        },
                },
            [0x4B] =
                {
                    .name = "*ALR imm.",
                    .micro_ops =
                        {
                            alr_imm,
                        },
                },
            [0x4C] =
                {
                    .name = "JMP $%04X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            jmp_abs_finish,
                        },
                },
            [0x4D] =
                {
                    .name = "EOR abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            eor_addr,
                        },
                },
            [0x4E] =
                {
                    .name = "LSR abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            lsr_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x4F] =
                {
                    .name = "*SRE abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            sre_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x50] =
                {
                    .name = "BVC",
                    .micro_ops =
                        {
                            bvc_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0x51] =
                {
                    .name = "EOR ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            eor_indexed_read_maybe_finish,
                            eor_indexed_reread_fixed,
                        },
                },
            [0x52] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x54] =
                {
                    .name = "*NOP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            nop_addr,
                        },
                },
            [0x55] =
                {
                    .name = "EOR zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            eor_addr,
                        },
                },
            [0x56] =
                {
                    .name = "LSR zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            lsr_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x57] =
                {
                    .name = "*SRE zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            sre_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x58] =
                {
                    .name = "CLI",
                    .micro_ops =
                        {
                            cli_imp,
                        },
                },
            [0x59] =
                {
                    .name = "EOR abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            eor_indexed_read_maybe_finish,
                            eor_indexed_reread_fixed,
                        },
                },
            [0x5A] =
                {
                    .name = "*NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0x5C] =
                {
                    .name = "*NOP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            nop_indexed_read_maybe_finish,
                            nop_addr,
                        },
                },
            [0x5D] =
                {
                    .name = "EOR abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            eor_indexed_read_maybe_finish,
                            eor_indexed_reread_fixed,
                        },
                },
            [0x5E] =
                {
                    .name = "LSR abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            dummy_temp_addr_read,
                            read_addr_data,
                            lsr_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x60] =
                {
                    .name = "RTS",
                    .micro_ops =
                        {
                            dummy_pc_read,
                            inc_sp,
                            pull_pc_low,
                            pull_pc_high_no_inc,
                            rts_finish,
                        },
                },
            [0x61] =
                {
                    .name = "ADC indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            adc_addr,
                        },
                },
            [0x62] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x63] =
                {
                    .name = "*RRA indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            read_addr_data,
                            rra_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x64] =
                {
                    .name = "*NOP zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            nop_addr,
                        },
                },
            [0x65] =
                {
                    .name = "ADC zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            adc_addr,
                        },
                },
            [0x66] =
                {
                    .name = "ROR zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            ror_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x67] =
                {
                    .name = "*RRA zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            rra_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x68] =
                {
                    .name = "PLA",
                    .micro_ops =
                        {
                            dummy_pc_read,
                            inc_sp,
                            pla_imp,
                        },
                },
            [0x69] =
                {
                    .name = "ADC imm.",
                    .micro_ops =
                        {
                            adc_imm,
                        },
                },
            [0x6A] =
                {
                    .name = "ROR acc.",
                    .micro_ops =
                        {
                            ror_acc,
                        },
                },
            [0x6B] =
                {
                    .name = "*ARR imm.",
                    .micro_ops =
                        {
                            arr_imm,
                        },
                },
            [0x6C] =
                {
                    .name = "JMP indirect",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            fetch_pointer_addr_low,
                            jmp_ind,
                        },
                },
            [0x6D] =
                {
                    .name = "ADC abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            adc_addr,
                        },
                },
            [0x6E] =
                {
                    .name = "ROR abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            ror_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x6F] =
                {
                    .name = "*RRA abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            rra_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x70] =
                {
                    .name = "BVS",
                    .micro_ops =
                        {
                            bvs_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0x71] =
                {
                    .name = "ADC ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            adc_indexed_read_maybe_finish,
                            adc_indexed_reread_fixed,
                        },
                },
            [0x72] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x74] =
                {
                    .name = "*NOP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            nop_addr,
                        },
                },
            [0x75] =
                {
                    .name = "ADC zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            adc_addr,
                        },
                },
            [0x76] =
                {
                    .name = "ROR zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            ror_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x77] =
                {
                    .name = "*RRA zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            rra_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x78] =
                {
                    .name = "SEI",
                    .micro_ops =
                        {
                            sei_imp,
                        },
                },
            [0x79] =
                {
                    .name = "ADC abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            adc_indexed_read_maybe_finish,
                            adc_indexed_reread_fixed,
                        },
                },
            [0x7A] =
                {
                    .name = "*NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0x7C] =
                {
                    .name = "*NOP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            nop_indexed_read_maybe_finish,
                            nop_addr,
                        },
                },
            [0x7D] =
                {
                    .name = "ADC abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            adc_indexed_read_maybe_finish,
                            adc_indexed_reread_fixed,
                        },
                },
            [0x7E] =
                {
                    .name = "ROR abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            dummy_temp_addr_read,
                            read_addr_data,
                            ror_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0x80] =
                {
                    .name = "*NOP imm",
                    .micro_ops =
                        {
                            nop_imm,
                        },
                },
            [0x81] =
                {
                    .name = "STA indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            sta_addr,
                        },
                },
            [0x82] =
                {
                    .name = "*NOP imm",
                    .micro_ops =
                        {
                            nop_imm,
                        },
                },
            [0x83] =
                {
                    .name = "*SAX indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            sax_addr,
                        },
                },
            [0x84] =
                {
                    .name = "STY zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            sty_addr,
                        },
                },
            [0x85] =
                {
                    .name = "STA zp.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            sta_addr,
                        },
                },
            [0x86] =
                {
                    .name = "STX zp.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            stx_addr,
                        },
                },
            [0x87] =
                {
                    .name = "*SAX zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            sax_addr,
                        },
                },
            [0x88] =
                {
                    .name = "DEY",
                    .micro_ops =
                        {
                            dey_imp,
                        },
                },
            [0x89] =
                {
                    .name = "*NOP imm",
                    .micro_ops =
                        {
                            nop_imm,
                        },
                },
            [0x8A] =
                {
                    .name = "TXA",
                    .micro_ops =
                        {
                            txa_imp,
                        },
                },
            [0x8B] =
                {
                    .name = "*XAA imm.",
                    .micro_ops =
                        {
                            xaa_imm,
                        },
                },
            [0x8C] =
                {
                    .name = "STY abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            sty_addr,
                        },
                },
            [0x8D] =
                {
                    .name = "STA $%04X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            sta_addr,
                        },
                },
            [0x8E] =
                {
                    .name = "STX abs.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            stx_addr,
                        },
                },
            [0x8F] =
                {
                    .name = "*SAX abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            sax_addr,
                        },
                },
            [0x90] =
                {
                    .name = "BCC",
                    .micro_ops =
                        {
                            bcc_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0x91] =
                {
                    .name = "STA indirect indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            dummy_temp_addr_read,
                            sta_addr,
                        },
                },
            [0x92] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0x94] =
                {
                    .name = "STY zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            sty_addr,
                        },
                },
            [0x95] =
                {
                    .name = "STA zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            sta_addr,
                        },
                },
            [0x96] =
                {
                    .name = "STX zp,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_y,
                            stx_addr,
                        },
                },
            [0x97] =
                {
                    .name = "*SAX zp,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_y,
                            sax_addr,
                        },
                },
            [0x98] =
                {
                    .name = "TYA",
                    .micro_ops =
                        {
                            tya_imp,
                        },
                },
            [0x99] =
                {
                    .name = "STA abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            sta_indexed_read_maybe_finish,
                            sta_indexed_rewrite_fixed,
                        },
                },
            [0x9A] =
                {
                    .name = "TXS",
                    .micro_ops =
                        {
                            txs_imp,
                        },
                },
            [0x9D] =
                {
                    .name = "STA abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            sta_indexed_read_maybe_finish,
                            sta_indexed_rewrite_fixed,
                        },
                },
            [0xA0] =
                {
                    .name = "LDY #$%02X",
                    .micro_ops =
                        {
                            ldy_imm,
                        },
                },
            [0xA1] =
                {
                    .name = "LDA indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            lda_addr,
                        },
                },
            [0xA2] =
                {
                    .name = "LDX #$%02X",
                    .micro_ops =
                        {
                            ldx_imm,
                        },
                },
            [0xA3] =
                {
                    .name = "*LAX indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            lax_addr,
                        },
                },
            [0xA4] =
                {
                    .name = "LDY zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            ldy_addr,
                        },
                },
            [0xA5] =
                {
                    .name = "LDA zp.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            lda_addr,
                        },
                },
            [0xA6] =
                {
                    .name = "LDX zp.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            ldx_addr,
                        },
                },
            [0xA7] =
                {
                    .name = "*LAX zp.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            lax_addr,
                        },
                },
            [0xA8] =
                {
                    .name = "TAY",
                    .micro_ops =
                        {
                            tay_imp,
                        },
                },
            [0xA9] =
                {
                    .name = "LDA #$%02X",
                    .micro_ops =
                        {
                            lda_imm,
                        },
                },
            [0xAA] =
                {
                    .name = "TAX",
                    .micro_ops =
                        {
                            tax_imp,
                        },
                },
            [0xAB] =
                {
                    .name = "*LAX imm.",
                    .micro_ops =
                        {
                            lax_imm,
                        },
                },
            [0xAC] =
                {
                    .name = "LDY abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            ldy_addr,
                        },
                },
            [0xAD] =
                {
                    .name = "LDA $%04X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            lda_addr,
                        },
                },
            [0xAE] =
                {
                    .name = "LDX abs.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            ldx_addr,
                        },
                },
            [0xAF] =
                {
                    .name = "LAX abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            lax_addr,
                        },
                },
            [0xB0] =
                {
                    .name = "BCS",
                    .micro_ops =
                        {
                            bcs_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0xB1] =
                {
                    .name = "LDA ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            lda_indexed_read_maybe_finish,
                            lda_indexed_reread_fixed,
                        },
                },
            [0xB2] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0xB4] =
                {
                    .name = "LDY zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            ldy_addr,
                        },
                },
            [0xB5] =
                {
                    .name = "LDA zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            lda_addr,
                        },
                },
            [0xB6] =
                {
                    .name = "LDX zp,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_y,
                            ldx_addr,
                        },
                },
            [0xB7] =
                {
                    .name = "*LAX zp,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_y,
                            lax_addr,
                        },
                },
            [0xB8] =
                {
                    .name = "CLV",
                    .micro_ops =
                        {
                            clv_imp,
                        },
                },
            [0xB9] =
                {
                    .name = "LDA abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            lda_indexed_read_maybe_finish,
                            lda_indexed_reread_fixed,
                        },
                },
            [0xBA] =
                {
                    .name = "TSX",
                    .micro_ops =
                        {
                            tsx_imp,
                        },
                },
            [0xBC] =
                {
                    .name = "LDY abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            ldy_indexed_read_maybe_finish,
                            ldy_indexed_reread_fixed,
                        },
                },
            [0xBD] =
                {
                    .name = "LDA abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            lda_indexed_read_maybe_finish,
                            lda_indexed_reread_fixed,
                        },
                },
            [0xBE] =
                {
                    .name = "LDX abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            ldx_indexed_read_maybe_finish,
                            ldx_indexed_reread_fixed,
                        },
                },
            [0xC0] =
                {
                    .name = "CPY imm.",
                    .micro_ops =
                        {
                            cpy_imm,
                        },
                },
            [0xC1] =
                {
                    .name = "CMP indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            cmp_addr,
                        },
                },
            [0xC2] =
                {
                    .name = "*NOP imm",
                    .micro_ops =
                        {
                            nop_imm,
                        },
                },
            [0xC3] =
                {
                    .name = "*DCP indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            read_addr_data,
                            dcp_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xC4] =
                {
                    .name = "CPY zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            cpy_addr,
                        },
                },
            [0xC5] =
                {
                    .name = "CMP zp.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            cmp_addr,
                        },
                },
            [0xC6] =
                {
                    .name = "DEC zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            dec_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xC7] =
                {
                    .name = "*DCP zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            dcp_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xC8] =
                {
                    .name = "INY",
                    .micro_ops =
                        {
                            iny_imp,
                        },
                },
            [0xC9] =
                {
                    .name = "CMP #$%02X",
                    .micro_ops =
                        {
                            cmp_imm,
                        },
                },
            [0xCA] =
                {
                    .name = "DEX",
                    .micro_ops =
                        {
                            dex_imp,
                        },
                },
            [0xCB] =
                {
                    .name = "*AXS imm.",
                    .micro_ops =
                        {
                            axs_imm,
                        },
                },
            [0xCC] =
                {
                    .name = "CPY abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            cpy_addr,
                        },
                },
            [0xCD] =
                {
                    .name = "CMP abs.",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            cmp_addr,
                        },
                },
            [0xCE] =
                {
                    .name = "DEC abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            dec_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xCF] =
                {
                    .name = "*DCP abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            dcp_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xD0] =
                {
                    .name = "BNE",
                    .micro_ops =
                        {
                            bne_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0xD1] =
                {
                    .name = "CMP ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            cmp_indexed_read_maybe_finish,
                            cmp_indexed_reread_fixed,
                        },
                },
            [0xD2] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0xD4] =
                {
                    .name = "*NOP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            nop_addr,
                        },
                },
            [0xD5] =
                {
                    .name = "CMP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            cmp_addr,
                        },
                },
            [0xD6] =
                {
                    .name = "DEC zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            dec_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xD7] =
                {
                    .name = "*DCP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            dcp_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xD8] =
                {
                    .name = "CLD",
                    .micro_ops =
                        {
                            cld_imp,
                        },
                },
            [0xD9] =
                {
                    .name = "CMP abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            cmp_indexed_read_maybe_finish,
                            cmp_indexed_reread_fixed,
                        },
                },
            [0xDA] =
                {
                    .name = "*NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0xDC] =
                {
                    .name = "*NOP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            nop_indexed_read_maybe_finish,
                            nop_addr,
                        },
                },
            [0xDD] =
                {
                    .name = "CMP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            cmp_indexed_read_maybe_finish,
                            cmp_indexed_reread_fixed,
                        },
                },
            [0xDE] =
                {
                    .name = "DEC abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            dummy_temp_addr_read,
                            read_addr_data,
                            dec_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xE0] =
                {
                    .name = "CPX imm.",
                    .micro_ops =
                        {
                            cpx_imm,
                        },
                },
            [0xE1] =
                {
                    .name = "SBC indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            sbc_addr,
                        },
                },
            [0xE2] =
                {
                    .name = "*NOP imm",
                    .micro_ops =
                        {
                            nop_imm,
                        },
                },
            [0xE3] =
                {
                    .name = "*ISC indexed ind.",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_add_x,
                            read_ptr_addr_low,
                            read_ptr_addr_high,
                            read_addr_data,
                            isc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xE4] =
                {
                    .name = "CPX zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            cpx_addr,
                        },
                },
            [0xE5] =
                {
                    .name = "SBC zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            sbc_addr,
                        },
                },
            [0xE6] =
                {
                    .name = "INC zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            inc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xE7] =
                {
                    .name = "*ISC zp",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_addr_data,
                            isc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xE8] =
                {
                    .name = "INX",
                    .micro_ops =
                        {
                            inx_imp,
                        },
                },
            [0xE9] =
                {
                    .name = "SBC imm",
                    .micro_ops =
                        {
                            sbc_imm,
                        },
                },
            [0xEA] =
                {
                    .name = "NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0xEB] =
                {
                    .name = "*USBC imm",
                    .micro_ops =
                        {
                            sbc_imm,
                        },
                },
            [0xEC] =
                {
                    .name = "CPX abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            cpx_addr,
                        },
                },
            [0xED] =
                {
                    .name = "SBC abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            sbc_addr,
                        },
                },
            [0xEE] =
                {
                    .name = "INC abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            inc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xEF] =
                {
                    .name = "*ISC abs",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high,
                            read_addr_data,
                            isc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xF0] =
                {
                    .name = "BEQ",
                    .micro_ops =
                        {
                            beq_fetch_offset,
                            branch_op,
                            branch_page_fix,
                        },
                },
            [0xF1] =
                {
                    .name = "SBC ind. indexed",
                    .micro_ops =
                        {
                            fetch_ptr,
                            read_ptr_addr_low,
                            read_ptr_addr_high_add_y,
                            sbc_indexed_read_maybe_finish,
                            sbc_indexed_reread_fixed,
                        },
                },
            [0xF2] =
                {
                    .name = "*KIL",
                    .micro_ops =
                        {
                            kil_imp,
                        },
                },
            [0xF4] =
                {
                    .name = "*NOP zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            nop_addr,
                        },
                },
            [0xF5] =
                {
                    .name = "SBC zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            sbc_addr,
                        },
                },
            [0xF6] =
                {
                    .name = "INC zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            inc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xF7] =
                {
                    .name = "*ISC zp,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_low_add_x,
                            read_addr_data,
                            isc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
            [0xF8] =
                {
                    .name = "SED",
                    .micro_ops =
                        {
                            sed_imp,
                        },
                },
            [0xF9] =
                {
                    .name = "SBC abs,Y",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_y,
                            sbc_indexed_read_maybe_finish,
                            sbc_indexed_reread_fixed,
                        },
                },
            [0xFA] =
                {
                    .name = "*NOP",
                    .micro_ops =
                        {
                            nop_imp,
                        },
                },
            [0xFC] =
                {
                    .name = "*NOP abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            nop_indexed_read_maybe_finish,
                            nop_addr,
                        },
                },
            [0xFD] =
                {
                    .name = "SBC abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            sbc_indexed_read_maybe_finish,
                            sbc_indexed_reread_fixed,
                        },
                },
            [0xFE] =
                {
                    .name = "INC abs,X",
                    .micro_ops =
                        {
                            read_pc_addr_low,
                            read_pc_addr_high_add_x,
                            dummy_temp_addr_read,
                            read_addr_data,
                            inc_dummy_write_and_compute,
                            write_data_and_finish,
                        },
                },
};