#include <assert.h>
#include <lib6502/6502.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "test-machine.h"

#define LOAD_ADDR 0x0000
#define START_ADDR 0x0400

// Verify this for the exact Klaus listing file.
#define SUCCESS_PC 0x3469

#define MAX_STEPS 100000000

int main(void) {
  TestMachine machine;
  memset(&machine, 0, sizeof(machine));

  if (load_binary(&machine, "test/roms/klaus-functional.bin", LOAD_ADDR) != 0) {
    return 1;
  }

  CPU6502* cpu =
      cpu6502_create(CPU6502_VARIANT_NMOS, test_read, test_write, &machine);
  cpu6502_reset(cpu);

  for (uint64_t step = 0; step < MAX_STEPS; step++) {
    CPU6502State state = cpu6502_get_state(cpu);

    if (state.PC == SUCCESS_PC) {
      printf("PASS: Klaus test reached success PC $%04X\n", state.PC);
      return 0;
    }

    cpu6502_step(cpu);
  }

  CPU6502State state = cpu6502_get_state(cpu);
  fprintf(stderr, "FAIL: exceeded max steps, PC=$%04X\n", state.PC);
  return 1;
}