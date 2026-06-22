#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>
#include <lib6502/6502.h>

typedef enum {
    OPERAND_MEMORY,
    OPERAND_ACCUMULATOR,
} OperandType;

typedef struct {
    OperandType type;
    uint16_t addr;
    int page_crossed;
} Operand;

typedef int (*exec_fn)(cpu6502 *cpu, Operand op);
typedef Operand (*addr_fn)(cpu6502 *cpu);

typedef struct {
    const char *mnemonic;
    exec_fn execute;
    addr_fn address;
    const char *operand_fmt;
    uint8_t bytes;
    uint8_t cycles;
    int page_cross_penalty;
} Opcode;

extern const Opcode opcode_table[256];

#endif // OPCODES_H