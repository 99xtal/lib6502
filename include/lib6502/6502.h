#ifndef LIB6502_6502_H
#define LIB6502_6502_H

#include <stdint.h>

typedef uint8_t (*cpu6502_read_fn)(void *ctx, uint16_t address);
typedef void (*cpu6502_write_fn)(void *ctx, uint16_t address, uint8_t value);

typedef struct cpu6502 {
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t SP;
    uint16_t PC;
    uint8_t status;
    cpu6502_read_fn read;
    cpu6502_write_fn write;
    void *ctx;
} cpu6502;

void cpu6502_init(
    cpu6502 *cpu,
    cpu6502_read_fn read,
    cpu6502_write_fn write,
    void *ctx
);

void cpu6502_reset(cpu6502 *cpu);

void cpu6502_step(cpu6502 *cpu);

#endif // LIB6502_6502_H