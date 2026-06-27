#ifndef ADDRESSING_H
#define ADDRESSING_H

#include <lib6502/6502.h>
#include "operand.h"

typedef Operand (*addr_fn)(cpu6502 *cpu);

typedef enum {
    ADDR_IMM,
    ADDR_ABS,
    ADDR_ABS_X,
    ADDR_ABS_Y,
    ADDR_IND,
    ADDR_IND_X,
    ADDR_IND_Y,
    ADDR_ZP,
    ADDR_ZP_X,
    ADDR_ZP_Y,
    ADDR_IMP,
    ADDR_REL,
    ADDR_ACC,
} AddrMode;

typedef struct {
    AddrMode type;
    addr_fn address;
    uint8_t bytes;
    const char *format;
} AddressingMode;

extern const AddressingMode addr_modes[13];

#endif // ADDRESSING_H