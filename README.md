# lib6502

A lightweight MOS 6502 CPU emulation library written in C.

`lib6502` provides a standalone implementation of the 6502 processor, including instruction decoding, addressing modes, registers, flags, and CPU execution. It is designed to be embedded into larger emulators by allowing the host application to provide memory and I/O through read/write callbacks.

## Features

- Complete implementation of official 6502 instructions
- Decimal mode (BCD) arithmetic support
- User-defined memory bus via read/write callbacks
- Instruction stepping with cycle counts
- Passes the Klaus Dormann 6502 functional test

## Building

Build the static library:

```sh
make
```

The resulting library is created at:

```
build/lib6502.a
```

Run the test suite:

```sh
make test
```

## Usage

The CPU does not contain memory internally. Instead, the host application provides callback functions that implement the system bus.

```c
#include <lib6502/6502.h>

uint8_t read(void *ctx, uint16_t addr) {
    return ((uint8_t *)ctx)[addr];
}

void write(void *ctx, uint16_t addr, uint16_t value) {
    ((uint8_t *)ctx)[addr] = value;
}

int main() {
    uint8_t memory[0x10000] = {0};

    cpu6502 cpu;

    cpu6502_init(&cpu, memory, read, write);
    cpu6502_reset(&cpu);

    while (1) {
        int cycles = cpu6502_step(&cpu);
    }
}
```

## Design

`lib6502` emulates only the CPU. It does not implement RAM, ROM, graphics, audio, or peripherals.

A complete system emulator should provide a bus implementation that maps memory accesses to the appropriate hardware components.
