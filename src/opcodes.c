#include "addressing.h"
#include "flags.h"
#include "instructions.h"
#include "opcodes.h"

enum OpcodeValue {
  OP_LDA_INDIRECT_X  = 0xA1,
  OP_LDA_IMM         = 0xA9,
  OP_LDA_ZERO_PAGE   = 0xA5,
  OP_LDA_ABS         = 0xAD,
  OP_LDA_INDIRECT_Y  = 0xB1,
  OP_LDA_ZERO_PAGE_X = 0xB5,
  OP_LDA_ABS_X       = 0xBD,
  OP_LDA_ABS_Y       = 0xB9,
  OP_NOP             = 0xEA,
};

const Opcode opcode_table[256] = {
  [OP_LDA_INDIRECT_X] = { .execute = lda, .address = addr_indirect_x, .cycles = 6, .page_cross_penalty = 0 },
  [OP_LDA_ABS_Y] = { .execute = lda, .address = addr_abs_y, .cycles = 3, .page_cross_penalty = 1 },
  [OP_LDA_IMM] = { .execute = lda, .address = addr_imm, .cycles = 2, .page_cross_penalty = 0 },
  [OP_LDA_ABS] = { .execute = lda, .address = addr_abs, .cycles = 4, .page_cross_penalty = 0 },
  [OP_LDA_INDIRECT_Y] = { .execute = lda, .address = addr_indirect_y, .cycles = 5, .page_cross_penalty = 0 },
  [OP_LDA_ABS_X] = { .execute = lda, .address = addr_abs_x, .cycles = 4, .page_cross_penalty = 1 },
  [OP_LDA_ZERO_PAGE] = { .execute = lda, .address = addr_zero_page, .cycles = 3, .page_cross_penalty = 0 },
  [OP_LDA_ZERO_PAGE_X] = { .execute = lda, .address = addr_zero_page_x, .cycles = 4, .page_cross_penalty = 0 },
  [OP_NOP] = { .execute = nop, .address = addr_implied, .cycles = 2, .page_cross_penalty = 0 }
};