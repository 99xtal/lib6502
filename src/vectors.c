#include <lib6502/6502.h>

uint16_t read_vector(cpu6502 *cpu, uint16_t vector_address) {
  uint8_t low = cpu->read(cpu->ctx, vector_address);
  uint8_t high = cpu->read(cpu->ctx, vector_address + 1);

  uint16_t result = ((uint16_t)high << 8 | low);
  return result;
}
