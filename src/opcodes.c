#include "addressing.h"
#include "flags.h"
#include "instructions.h"
#include "opcodes.h"

const Opcode opcode_table[256] = {
  [0x18] = { .execute = clc, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0x38] = { .execute = sec, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0x58] = { .execute = cli, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0x78] = { .execute = sei, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0xA1] = { .execute = lda, .address = addr_indirect_x, .cycles = 6, .page_cross_penalty = 0 },
  [0xA5] = { .execute = lda, .address = addr_zero_page, .cycles = 3, .page_cross_penalty = 0 },
  [0xA9] = { .execute = lda, .address = addr_imm, .cycles = 2, .page_cross_penalty = 0 },
  [0xAD] = { .execute = lda, .address = addr_abs, .cycles = 4, .page_cross_penalty = 0 },
  [0xB1] = { .execute = lda, .address = addr_indirect_y, .cycles = 5, .page_cross_penalty = 0 },
  [0xB5] = { .execute = lda, .address = addr_zero_page_x, .cycles = 4, .page_cross_penalty = 0 },
  [0xB8] = { .execute = clv, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0xB9] = { .execute = lda, .address = addr_abs_y, .cycles = 3, .page_cross_penalty = 1 },
  [0xBD] = { .execute = lda, .address = addr_abs_x, .cycles = 4, .page_cross_penalty = 1 },
  [0xD8] = { .execute = cld, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0xEA] = { .execute = nop, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
  [0xF8] = { .execute = sed, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 },
};