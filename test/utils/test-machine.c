#include <stdio.h>

#include "test-machine.h"

uint8_t test_read(void *ctx, uint16_t addr) {
  TestMachine *m = ctx;
  return m->mem[addr];
}

void test_write(void *ctx, uint16_t addr, uint8_t value) {
  TestMachine *m = ctx;
  m->mem[addr] = value;
}

int load_binary(TestMachine *m, const char *path, uint16_t address) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    perror("fopen");
    return -1;
  }

  size_t bytes_read = fread(
    &m->mem[address],
    1,
    sizeof(m->mem) - address,
    f
  );

  if (ferror(f)) {
    perror("fread");
    fclose(f);
    return -1;
  }

  fclose(f);

  printf("Loaded %zu bytes at $%04X\n", bytes_read, address);

  return 0;
}