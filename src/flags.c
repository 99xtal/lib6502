#include "flags.h"

#include "6502.h"

void set_flag(CPU6502* cpu, StatusFlag flag, uint8_t value) {
  if (value)
    cpu->status |= flag;
  else
    cpu->status &= ~flag;
}

uint8_t get_flag(CPU6502* cpu, StatusFlag flag) {
  return (cpu->status & flag) != 0;
}