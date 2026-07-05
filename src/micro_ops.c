#include "micro_ops.h"

#include <string.h>

#include "6502.h"
#include "flags.h"
#include "stack.h"

uint8_t read(CPU6502* cpu, uint16_t addr) { return cpu->read(cpu->ctx, addr); }

uint16_t full_addr(CPU6502* cpu) {
  return (cpu->op.addr_hi << 8) | cpu->op.addr_lo;
}

void finish_op(CPU6502* cpu) { memset(&cpu->op, 0, sizeof(cpu->op)); }

void dummy_read(CPU6502* cpu, uint16_t addr) { read(cpu, addr); }

void inc_sp(CPU6502* cpu) { cpu->SP++; }

void dummy_pc_read(CPU6502* cpu) { read(cpu, cpu->PC++); }

void set_abs_addr_low(CPU6502* cpu) { cpu->op.addr_lo = read(cpu, cpu->PC++); }

void set_abs_addr_high(CPU6502* cpu) { cpu->op.addr_hi = read(cpu, cpu->PC++); }

void push_pc_high(CPU6502* cpu) { stack_push_u8(cpu, (cpu->PC >> 8) & 0xFF); }

void push_pc_low(CPU6502* cpu) { stack_push_u8(cpu, cpu->PC & 0xFF); }

void pull_pc_low(CPU6502* cpu) {
  cpu->op.addr_lo = cpu->read(cpu->ctx, 0x0100 | cpu->SP);
  cpu->SP++;
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

void pull_p(CPU6502* cpu) {
  cpu->status = cpu->read(cpu->ctx, 0x0100 | cpu->SP);
  cpu->SP++;
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

void ldy_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);
  cpu->Y = value;
  set_nz(cpu, value);
  finish_op(cpu);
}

void lda_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);

  cpu->A = value;
  set_nz(cpu, value);

  finish_op(cpu);
}

void lda_abs(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  uint8_t value = read(cpu, addr);

  cpu->A = value;
  set_nz(cpu, value);

  finish_op(cpu);
}

void sta_abs(CPU6502* cpu) {
  uint16_t addr = full_addr(cpu);
  cpu->write(cpu->ctx, addr, cpu->A);

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

/**
 * Jumps & Calls
 */
void jmp_abs_finish(CPU6502* cpu) {
  cpu->op.addr_hi = read(cpu, cpu->PC);
  uint16_t addr = full_addr(cpu);

  cpu->PC = addr;

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

const OpDef instruction_defs[256] = {
    [0x00] =
        {
            .name = "BRK",
            .micro_ops =
                {
                    dummy_pc_read,
                    push_pc_high,
                    push_pc_low,
                    push_p_with_break,
                    read_irq_low,
                    read_irq_high_finish,
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
    [0x18] =
        {
            .name = "CLC",
            .micro_ops =
                {
                    clc_imp,
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
    [0x40] =
        {
            .name = "RTI",
            .micro_ops =
                {
                    dummy_pc_read,
                    inc_sp,
                    pull_p,
                    pull_pc_low,
                    pull_pc_high_finish,
                },
        },
    [0x4C] =
        {
            .name = "JMP $%04X",
            .micro_ops =
                {
                    set_abs_addr_low,
                    jmp_abs_finish,
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
    [0x78] =
        {
            .name = "SEI",
            .micro_ops =
                {
                    sei_imp,
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
    [0x8A] =
        {
            .name = "TXA",
            .micro_ops =
                {
                    txa_imp,
                },
        },
    [0x8D] =
        {
            .name = "STA $%04X",
            .micro_ops =
                {
                    set_abs_addr_low,
                    set_abs_addr_high,
                    sta_abs,
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
    [0x9A] =
        {
            .name = "TXS",
            .micro_ops =
                {
                    txs_imp,
                },
        },
    [0xA0] = {.name = "LDY #$%02X",
              .micro_ops =
                  {
                      ldy_imm,
                  }},
    [0xA2] =
        {
            .name = "LDX #$%02X",
            .micro_ops =
                {
                    ldx_imm,
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
    [0xAD] =
        {
            .name = "LDA $%04X",
            .micro_ops =
                {
                    set_abs_addr_low,
                    set_abs_addr_high,
                    lda_abs,
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
    [0xD8] =
        {
            .name = "CLD",
            .micro_ops =
                {
                    cld_imp,
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
    [0xE8] =
        {
            .name = "INX",
            .micro_ops =
                {
                    inx_imp,
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
    [0xF8] =
        {
            .name = "SED",
            .micro_ops =
                {
                    sed_imp,
                },
        },
};