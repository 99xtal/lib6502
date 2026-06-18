#ifndef FLAGS_H
#define FLAGS_H

#include <stdint.h>
#include <lib6502/6502.h>

typedef enum {
  FLAG_CARRY             = 1 << 0,
  FLAG_ZERO              = 1 << 1,
  FLAG_INTERRUPT_DISABLE = 1 << 2,
  FLAG_DECIMAL_MODE      = 1 << 3,
  FLAG_BREAK             = 1 << 4,
  FLAG_UNUSED            = 1 << 5,
  FLAG_OVERFLOW          = 1 << 6,
  FLAG_NEGATIVE          = 1 << 7,
} StatusFlag;

void set_flag(cpu6502 *cpu, StatusFlag flag, uint8_t value);

#endif // FLAGS_H