#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <lib6502/6502.h>
#include "test-machine.h"

#define LOAD_ADDR  0x0000
#define START_ADDR 0x0400

// Verify this for the exact Klaus listing file.
#define SUCCESS_PC 0x347D

#define MAX_STEPS 100000000

int main(void) {
  TestMachine machine;
  memset(&machine, 0, sizeof(machine));

  if (load_binary(&machine, "build/test/roms/klaus-functional.bin", LOAD_ADDR) != 0) {
    return 1;
  }

  cpu6502 cpu;
  cpu6502_init(&cpu, test_read, test_write, &machine);
  cpu6502_reset(&cpu);

  cpu.PC = START_ADDR;

  for (uint64_t step = 0; step < MAX_STEPS; step++) {
    if (cpu.PC == SUCCESS_PC) {
      printf("PASS: Klaus test reached success PC $%04X\n", cpu.PC);
      return 0;
    }

    uint16_t old_pc = cpu.PC;

    int cycles = cpu6502_step(&cpu);
    if (cycles < 0) {
      fprintf(stderr, "CPU error at PC=$%04X\n", cpu.PC);
      return 1;
    }

    // Klaus failure traps often loop forever at a failing PC.
    if (cpu.PC == old_pc && cpu.PC != SUCCESS_PC) {
      fprintf(stderr, "FAIL: stuck at PC=$%04X\n", cpu.PC);
      return 1;
    }
  }

  fprintf(stderr, "FAIL: exceeded max steps, PC=$%04X\n", cpu.PC);
  return 1;
}