#ifndef LIB6502_6502_H
#define LIB6502_6502_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum {
  // documented instructions only
  CPU6502_VARIANT_STRICT,
  // documented and undocumented instructions of the original MOS 6502
  CPU6502_VARIANT_NMOS,
  // variant produced by Ricoh for the NES (NTSC version)
  CPU6502_VARIANT_RP2A03,
} CPU6502Variant;

typedef uint8_t (*CPU6502ReadFn)(void* ctx, uint16_t address);
typedef void (*CPU6502WriteFn)(void* ctx, uint16_t address, uint8_t value);

typedef struct CPU6502 CPU6502;

CPU6502* cpu6502_create(CPU6502Variant variant, CPU6502ReadFn read,
                        CPU6502WriteFn write, void* ctx);

int cpu6502_reset(CPU6502* cpu);

int cpu6502_step(CPU6502* cpu);

int cpu6502_nmi(CPU6502* cpu);

void cpu6502_destroy(CPU6502* cpu);

typedef struct CPU6502State {
  uint8_t A;
  uint8_t X;
  uint8_t Y;
  uint8_t SP;
  uint16_t PC;
  uint8_t status;
} CPU6502State;

CPU6502State cpu6502_get_state(CPU6502* cpu);

int cpu6502_set_pc(CPU6502* cpu, uint16_t addr);

#endif  // LIB6502_6502_H