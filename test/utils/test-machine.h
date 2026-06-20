#ifndef TEST_MACHINE_H
#define TEST_MACHINE_H

#include <stdint.h>

typedef struct {
  uint8_t mem[0xFFFF];
} TestMachine;

uint8_t test_read(void *ctx, uint16_t addr);

void test_write(void *ctx, uint16_t addr, uint8_t value);

int load_binary(TestMachine *m, const char *path, uint16_t address);

#endif // TEST_MACHINE_H