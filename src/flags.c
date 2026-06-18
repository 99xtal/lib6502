#include <lib6502/6502.h>

#include "flags.h"

void set_flag(cpu6502 *cpu, StatusFlag flag, uint8_t value) {
  if (value)
    cpu->status |= flag;
  else
    cpu->status &= ~flag;
}

uint8_t get_flag(cpu6502 *cpu, StatusFlag flag) {
  return (cpu->status & flag) != 0;
}