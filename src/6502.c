#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <lib6502/6502.h>

#include "flags.h"
#include "opcodes.h"
#include "vectors.h"

void cpu6502_init(cpu6502 *cpu, cpu6502_read_fn read, cpu6502_write_fn write, void *ctx) {
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->SP = 0;
    cpu->status = 0 | FLAG_UNUSED; // Set unused flag to 1

    cpu->read = read;
    cpu->write = write;
    cpu->ctx = ctx;
}

int cpu6502_reset(cpu6502 *cpu) {
    int clock_cycles = 7;
    uint16_t reset_position = read_vector(cpu, VECTOR_RESET);

    cpu->SP = 0xFD; // Reset stack pointer to 0x01FD
    cpu->PC = reset_position;

    return clock_cycles;
}

int cpu6502_step(cpu6502 *cpu) {
    uint8_t opcode_byte = cpu->read(cpu->ctx, cpu->PC);
    Opcode opcode = opcode_table[opcode_byte];
    uint8_t bytes[3];

    if (cpu->trace) {
        for (uint8_t i = 0; i < opcode.bytes; i++) {
            uint16_t p = cpu->PC + i;
            bytes[i] = cpu->read(cpu->ctx, p);
        }
    }

    cpu->PC++;

    Operand op = opcode.address(cpu);
    int extra_cycles = opcode.execute(cpu, op);

    int cycles = opcode.cycles + extra_cycles;

    if (op.page_crossed) {
        cycles += opcode.page_cross_penalty;
    }

    if (cpu->trace) {
        cpu6502_trace t = {
            .PC = cpu->PC,
            .bytes_count = opcode.bytes,
            .mnemonic = opcode.mnemonic,
            .A = cpu->A,
            .X = cpu->X,
            .Y = cpu->Y,
            .status = cpu->status,
            .SP = cpu->SP,
            .cycles = cycles,
        };
        memcpy(t.bytes, bytes, sizeof bytes);

        if (opcode.operand_fmt && opcode.operand_fmt[0] != '\0') {
            if (opcode.bytes == 2) {
                snprintf(t.operand, sizeof t.operand,
                        opcode.operand_fmt,
                        bytes[1]);
            } else if (opcode.bytes == 3) {
                uint16_t operand =
                    (uint16_t)bytes[1] |
                    ((uint16_t)bytes[2] << 8);

                snprintf(t.operand, sizeof t.operand,
                        opcode.operand_fmt,
                        operand);
            }
        }

        cpu->trace(cpu->trace_ctx, t);
    }

    return cycles;
}