#ifndef LIB6502_6502_H
#define LIB6502_6502_H

#include <stdint.h>
#include <stdlib.h>

typedef enum {
    // documented instructions only
    CPU6502_VARIANT_STRICT,
    // documented and undocumented instructions
    CPU6502_VARIANT_NMOS,
} cpu6502_variant;

typedef struct cput6502_trace {
    uint16_t PC;
    uint8_t bytes[3];
    size_t bytes_count;
    const char *mnemonic;
    char operand[25];
    uint8_t SP;
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t status;
    uint8_t cycles;
} cpu6502_trace;

typedef uint8_t (*cpu6502_read_fn)(void *ctx, uint16_t address);
typedef void (*cpu6502_write_fn)(void *ctx, uint16_t address, uint8_t value);
typedef void (*cpu6502_trace_fn)(void *trace_ctx, cpu6502_trace trace);

typedef struct cpu6502 {
    cpu6502_variant variant;
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t SP;
    uint16_t PC;
    uint8_t status;
    cpu6502_read_fn read;
    cpu6502_write_fn write;
    void *ctx;

    // Optional tracing
    cpu6502_trace_fn trace;
    void *trace_ctx;
} cpu6502;

void cpu6502_init(
    cpu6502 *cpu,
    cpu6502_variant variant,
    cpu6502_read_fn read,
    cpu6502_write_fn write,
    void *ctx
);

int cpu6502_reset(cpu6502 *cpu);

int cpu6502_step(cpu6502 *cpu);

int cpu6502_nmi(cpu6502 *cpu);

#endif // LIB6502_6502_H