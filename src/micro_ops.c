#include "micro_ops.h"

#include <string.h>

#include "6502.h"
#include "flags.h"

void finish_op(CPU6502* cpu) { memset(&cpu->op, 0, sizeof(cpu->op)); }

void dummy_read(CPU6502* cpu, uint16_t addr) { cpu->read(cpu->ctx, addr); }

uint8_t read(CPU6502* cpu, uint16_t addr) { return cpu->read(cpu->ctx, addr); }

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

void lda_imm(CPU6502* cpu) {
  uint8_t value = read(cpu, cpu->PC++);
  cpu->A = value;
  set_nz(cpu, value);
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
    [0x9A] =
        {
            .name = "TXS",
            .micro_ops =
                {
                    txs_imp,
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
    [0xA9] =
        {
            .name = "LDA #$%02X",
            .micro_ops =
                {
                    lda_imm,
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
    [0xD8] =
        {
            .name = "CLD",
            .micro_ops =
                {
                    cld_imp,
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