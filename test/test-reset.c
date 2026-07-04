#include <assert.h>
#include <lib6502/6502.h>
#include <stdint.h>

struct test_bus {
  uint8_t mem[65536];
};

static uint8_t bus_read(void* ctx, uint16_t address) {
  struct test_bus* bus = (struct test_bus*)ctx;
  return bus->mem[address];
}

static void bus_write(void* ctx, uint16_t address, uint8_t value) {
  struct test_bus* bus = (struct test_bus*)ctx;
  bus->mem[address] = value;
}

int main(void) {
  struct test_bus bus = {0};
  CPU6502* cpu =
      cpu6502_create(CPU6502_VARIANT_STRICT, bus_read, bus_write, &bus);

  bus.mem[0xFFFC] = 0x34;
  bus.mem[0xFFFD] = 0x12;

  cpu6502_reset(cpu);

  CPU6502State state = cpu6502_get_state(cpu);

  assert(state.SP == 0xFD);
  assert(state.PC == 0x1234);

  return 0;
}