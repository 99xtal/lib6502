#include <stdio.h>

#include "test-machine.h"

uint8_t test_read(void *ctx, uint16_t addr) {
  TestMachine *m = ctx;
  return m->mem[addr];
}

void test_write(void *ctx, uint16_t addr, uint8_t value) {
  TestMachine *m = ctx;
  
  if (addr >= 0x8000) {
    return;
  }

  m->mem[addr] = value;
}

int load_rom(TestMachine *m, const char *path) {
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