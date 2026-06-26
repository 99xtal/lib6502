#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>
#include <lib6502/6502.h>

#include "addressing.h"
#include "operand.h"

typedef int (*exec_fn)(cpu6502 *cpu, Operand op);
typedef struct {
    const char *mnemonic;
    AddrMode addr_mode;
    exec_fn execute;
    uint8_t cycles;
    int page_cross_penalty;
} Opcode;

extern const Opcode opcode_table_strict[256];
extern const Opcode opcode_table_nmos[256];

#endif // OPCODES_H