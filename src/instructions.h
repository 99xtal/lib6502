#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <lib6502/6502.h>

#include "operand.h"

typedef int (*exec_fn)(cpu6502 *cpu, Operand op);

typedef enum Inst {
  /** Load/Store Operations */
  INST_LDA,
  INST_LDX,
  INST_LDY,
  INST_STA,
  INST_STX,
  INST_STY,

  /** Register Transfers */
  INST_TAX,
  INST_TAY,
  INST_TXA,
  INST_TYA,

  /** Stack Operations */
  INST_TSX,
  INST_TXS,
  INST_PHA,
  INST_PHP,
  INST_PLA,
  INST_PLP,

  /** Logical */
  INST_AND,
  INST_EOR,
  INST_ORA,
  INST_BIT,

  /** Arithmetic */
  INST_ADC,
  INST_SBC,
  INST_CMP,
  INST_CPX,
  INST_CPY,

  /** Increments & Decrements */
  INST_INC,
  INST_INX,
  INST_INY,
  INST_DEC,
  INST_DEX,
  INST_DEY,

  /** Shifts */
  INST_ASL,
  INST_LSR,
  INST_ROL,
  INST_ROR,

  /** Jumps & Calls */
  INST_JMP,
  INST_JSR,
  INST_RTS,

  /** Branches */
  INST_BCC,
  INST_BCS,
  INST_BEQ,
  INST_BMI,
  INST_BNE,
  INST_BPL,
  INST_BVC,
  INST_BVS,

  /** Status Flag Changes */
  INST_CLC,
  INST_CLD,
  INST_CLI,
  INST_CLV,
  INST_SEC,
  INST_SED,
  INST_SEI,

  /** System Functions */
  INST_BRK,
  INST_NOP,
  INST_RTI,

  /** Undocumented Instructions */
  INST_KIL,
  INST_SLO,
  INST_ANC,
  INST_RLA,
  INST_SRE,
  INST_ALR,
  INST_RRA,
} Inst;

typedef struct {
  exec_fn execute;
  const char *mnemonic;
} Instruction;

extern const Instruction instructions[];

#endif // INSTRUCTIONS_H