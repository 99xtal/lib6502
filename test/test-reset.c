#include <assert.h>
#include <stdint.h>
#include <lib6502/6502.h>

struct test_bus {
    uint8_t mem[65536];
};

static uint8_t bus_read(void *ctx, uint16_t address) {
    struct test_bus *bus = (struct test_bus *)ctx;
    return bus->mem[address];
}

static void bus_write(void *ctx, uint16_t address, uint8_t value) {
    struct test_bus *bus = (struct test_bus *)ctx;
    bus->mem[address] = value;
}

int main(void) {
    struct test_bus bus = {0};
    cpu6502 cpu = {0};

    bus.mem[0xFFFC] = 0x34;
    bus.mem[0xFFFD] = 0x12;

    cpu6502_init(&cpu, bus_read, bus_write, &bus);
    cpu6502_reset(&cpu);

    assert(cpu.SP == 0xFD);
    assert(cpu.PC == 0x1234);

    return 0;
}