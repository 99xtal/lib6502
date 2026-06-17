#include <stdint.h>
#include <lib6502/6502.h>

/** Processor status flags */
#define FLAG_CARRY (1 << 0)                 // Set if the last operation resulted in a carry out of the most significant bit
#define FLAG_ZERO (1 << 1)                  // Set if the last operation resulted in a zero value
#define FLAG_INTERRUPT_DISABLE (1 << 2)     // Set if interrupts are disabled
#define FLAG_DECIMAL_MODE (1 << 3)          // Set if the CPU is in decimal mode (BCD)
#define FLAG_BREAK (1 << 4)                 // Set if the last operation was a BRK instruction
#define FLAG_UNUSED (1 << 5)                // Unused flag, always set
#define FLAG_OVERFLOW (1 << 6)              // Set if the last operation resulted in an overflow
#define FLAG_NEGATIVE (1 << 7)              // Set if the last operation resulted in a negative value (most significant bit set)

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

void cpu6502_reset(cpu6502 *cpu) {
    uint16_t reset_position = read_vector(cpu, VECTOR_RESET_LOW);

    cpu->SP = 0xFD; // Reset stack pointer to 0x01FD
    cpu->PC = reset_position;
}

void cpu6502_step(cpu6502 *cpu) {
    uint8_t opcode = cpu->read(cpu->ctx, cpu->PC);

    // For now, we will just increment the program counter to simulate a NOP instruction
    cpu->PC += 1;
}

void set_flag(cpu6502 *cpu, uint8_t flag, uint8_t value) {
    if (value) {
        cpu->status |= flag;
    } else {
        cpu->status &= ~flag;
    }
}