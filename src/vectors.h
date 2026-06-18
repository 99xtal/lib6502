#ifndef VECTORS_H
#define VECTORS_H

#include <lib6502/6502.h>

typedef enum {
  VECTOR_NMI = 0xFFFA,    // Address of the low byte of the non-maskable interrupt vector
  VECTOR_RESET = 0xFFFC,  // Address of the low byte of the reset vector
  VECTOR_IRQ = 0xFFFE,    // Address of the low byte of the IRQ/BRK vector
} Vectors;

uint16_t read_vector(cpu6502 *cpu, uint16_t vector_address);

#endif // VECTORS_H