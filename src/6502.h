#ifndef CPU6502_INTERNAL_H
#define CPU6502_INTERNAL_H

#include <lib6502/6502.h>
#include <stdint.h>

typedef void (*MicroOpFn)(CPU6502* cpu);

typedef struct OpDef {
  const char* name;
  MicroOpFn micro_ops[8];
} OpDef;

typedef struct OpState {
  OpDef* def;
  uint8_t cycle;
  uint8_t addr_lo;
  uint8_t addr_hi;
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