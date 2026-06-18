#include <stdint.h>
#include <lib6502/6502.h>

#include "flags.h"
#include "opcodes.h"

/** Memory vector addresses */
#define VECTOR_NMI_LOW 0xFFFA               // Address of the low byte of the non-maskable interrupt vector
#define VECTOR_NMI_HIGH 0xFFFB              // Address of the high byte of the NMI vector
#define VECTOR_RESET_LOW 0xFFFC             // Address of the low byte of the reset vector
#define VECTOR_RESET_HIGH 0xFFFD            // Address of the high byte of the reset vector
#define VECTOR_IRQ_LOW 0xFFFE               // Address of the low byte of the IRQ/BRK vector
#define VECTOR_IRQ_HIGH 0xFFFF              // Address of the high byte of the IRQ/BRK vector

uint16_t read_vector(cpu6502 *cpu, uint16_t vector_address) {
    uint8_t low_byte = cpu->read(cpu->ctx, vector_address);
    uint8_t high_byte = cpu->read(cpu->ctx, vector_address + 1);
    return ((uint16_t)high_byte << 8) | low_byte;
}

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
    uint16_t reset_position = read_vector(cpu, VECTOR_RESET_LOW);

    cpu->SP = 0xFD; // Reset stack pointer to 0x01FD
    cpu->PC = reset_position;

    return clock_cycles;
}

int cpu6502_step(cpu6502 *cpu) {
    uint8_t opcode_byte = cpu->read(cpu->ctx, cpu->PC);
    cpu->PC++;

    Opcode opcode = opcode_table[opcode_byte];

    Operand op = opcode.address(cpu);
    int extra_cycles = opcode.execute(cpu, op);

    int cycles = opcode.cycles + extra_cycles;

    if (op.page_crossed) {
        cycles += opcode.page_cross_penalty;
    }

    return cycles;
}