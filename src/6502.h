#ifndef CPU6502_INTERNAL_H
#define CPU6502_INTERNAL_H

#include <lib6502/6502.h>
#include <stdint.h>

#include "micro_ops.h"

typedef struct OpState {
  OpDef* def;
  uint8_t cycle;
  uint8_t addr_lo;
  uint8_t addr_hi;
  int8_t offset;
  uint16_t old_pc;
  uint16_t ptr;
  bool page_crossed;
  uint16_t addr;
  uint16_t temp_addr;
} OpState;

struct CPU6502 {
  CPU6502Variant variant;
  uint8_t A;
  uint8_t X;
  uint8_t Y;
  uint8_t SP;
  uint16_t PC;
  uint8_t status;
  CPU6502ReadFn read;
  CPU6502WriteFn write;
  void* ctx;

  // stops program execution, set by undocumented *KIL opcode
  bool jammed;

  // interrupt flags
  bool reset_requested;

  OpState op;
};

#endif