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

typedef struct CPU6502Trace {
  uint16_t PC;
  uint8_t bytes[3];
  size_t bytes_count;
  bool is_undocumented_inst;
  const char* mnemonic;
  char operand[25];
  uint8_t SP;
  uint8_t A;
  uint8_t X;
  uint8_t Y;

  /**
   * Status Register (P)
   *
   * 7  bit  0
   * ---- ----
   * NV1B DIZC
   * |||| ||||
   * |||| |||+- Carry
   * |||| ||+-- Zero
   * |||| |+--- Interrupt Disable
   * |||| +---- Decimal
   * |||+------ (No CPU effect; see: the B flag)
   * ||+------- (No CPU effect; always pushed as 1)
   * |+-------- Overflow
   * +--------- Negative
   */
  uint8_t status;
  uint8_t cycles;
} CPU6502Trace;

typedef uint8_t (*CPU6502ReadFn)(void* ctx, uint16_t address);
typedef void (*CPU6502WriteFn)(void* ctx, uint16_t address, uint8_t value);
typedef void (*CPU6502TraceFn)(void* trace_ctx, CPU6502Trace trace);

typedef struct CPU6502 {
  CPU6502Variant variant;
  uint8_t A;
  uint8_t X;
  uint8_t Y;
  uint8_t SP;
  uint16_t PC;
  uint8_t status;
  CPU6502ReadFn read;
  CPU6502WriteFn write;
  void* ctx;

  // stops program execution, set by undocumented *KIL opcode
  bool jammed;

  // Optional tracing
  CPU6502TraceFn trace;
  void* trace_ctx;
} CPU6502;

void cpu6502_init(CPU6502* cpu, CPU6502Variant variant, CPU6502ReadFn read,
                  CPU6502WriteFn write, void* ctx);

int cpu6502_reset(CPU6502* cpu);

int cpu6502_step(CPU6502* cpu);

int cpu6502_nmi(CPU6502* cpu);

#endif  // LIB6502_6502_H