#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <lib6502/6502.h>

#include "flags.h"
#include "opcodes.h"
#include "stack.h"
#include "trace.h"
#include "vectors.h"

void cpu6502_init(cpu6502 *cpu, cpu6502_variant variant, cpu6502_read_fn read, cpu6502_write_fn write, void *ctx) {
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->SP = 0;
    cpu->status = 0 | FLAG_UNUSED; // Set unused flag to 1
    cpu->variant = variant;

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
    if (cpu->jammed) {
        return 1; // burn a cycle
    }
    
    // choose which instruction set variant to use
    const Opcode *opcode_table;
    switch (cpu->variant) {
        case CPU6502_VARIANT_NMOS:
            opcode_table = opcode_table_nmos;
            break;
        case CPU6502_VARIANT_STRICT:
        default:
            opcode_table = opcode_table_strict;
    }

    uint16_t initial_pc = cpu->PC;

    // read opcode
    uint8_t opcode_byte = cpu->read(cpu->ctx, cpu->PC++);
    Opcode opcode = opcode_table[opcode_byte];
    AddressingMode addressing_mode = addr_modes[opcode.addr_mode];
    Instruction instruction = instructions[opcode.instruction];

    Operand op = addressing_mode.address(cpu);

    // build trace
    cpu6502_trace t = {0};
    uint8_t bytes[3];
    if (cpu->trace) {
        build_trace(
            &t,
            cpu,
            initial_pc,
            &op,
            &addressing_mode,
            &instruction
        );
    }

    // execute instruction
    int extra_cycles = instruction.execute(cpu, op);

    int cycles = opcode.cycles + extra_cycles;

    if (op.page_crossed) {
        cycles += opcode.page_cross_penalty;
    }

    if (cpu->trace) {
        t.cycles = cycles;

        cpu->trace(cpu->trace_ctx, t);
    }

    return cycles;
}

int cpu6502_nmi(cpu6502 *cpu) {
    stack_push_u16(cpu, cpu->PC);
    stack_push_u8(cpu, (cpu->status & ~FLAG_BREAK) | FLAG_UNUSED);

    set_flag(cpu, FLAG_INTERRUPT_DISABLE, 1);

    cpu->PC = read_vector(cpu, VECTOR_NMI);

    return 7;
}