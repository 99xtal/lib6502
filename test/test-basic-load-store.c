#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <lib6502/6502.h>

#define HALT_ADDR 0x00FF

typedef struct {
  uint8_t mem[0xFFFF];
} TestMachine;

static uint8_t test_read(void *ctx, uint16_t addr) {
  TestMachine *m = ctx;
  return m->mem[addr];
}

static void test_write(void *ctx, uint16_t addr, uint8_t value) {
  TestMachine *m = ctx;
  
  if (addr >= 0x8000) {
    return;
  }

  m->mem[addr] = value;
}

static int load_rom(TestMachine *m, const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    perror("fopen");
    return -1;
  }

  size_t n = fread(&m->mem[0x8000], 1, 0x8000, f);
  fclose(f);

  if (n != 0x8000) {
    fprintf(stderr, "expected 0x8000 bytes, got %zu\n", n);
    return -1;
  }

  return 0;
}

int main(void) {
  TestMachine machine;
  memset(&machine, 0, sizeof(machine));

  if (load_rom(&machine, "build/test/roms/basic-load-store.bin") != 0) {
    return 1;
  }

  cpu6502 cpu;
  cpu6502_init(&cpu, test_read, test_write, &machine);
  cpu6502_reset(&cpu);

  int total_cycles = 0;

  for (int i = 0; i < 1000; i++) {
    uint8_t opcode = machine.mem[cpu.PC];

    int cycles = cpu6502_step(&cpu);
    if (cycles < 0) {
      fprintf(stderr, "CPU error at PC=%04X\n", cpu.PC);
      return 1;
    }

    total_cycles += cycles;
    
    if (machine.mem[HALT_ADDR] == 1) {
      break;
    }
  }

  assert(machine.mem[0x0200] = 0x42);
  assert(machine.mem[0x0201] = 0x99);

  return 0;
}