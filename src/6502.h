#ifndef CPU6502_INTERNAL_H
#define CPU6502_INTERNAL_H

#include <lib6502/6502.h>
#include <stdint.h>

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
};

#endif