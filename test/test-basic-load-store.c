#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <lib6502/6502.h>
#include "test-machine.h"

#define HALT_ADDR 0x00FF

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