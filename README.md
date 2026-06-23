# lib6502

A lightweight MOS 6502 CPU emulation library written in C.

`lib6502` provides a standalone implementation of the 6502 processor, including instruction decoding, addressing modes, registers, flags, and CPU execution. It is designed to be embedded into larger emulators by allowing the host application to provide memory and I/O through read/write callbacks.

## Features

* Complete implementation of official MOS 6502 instructions
* Decimal mode (BCD) arithmetic support
* User-defined memory bus via read/write callbacks
* Instruction stepping with cycle counts
* Optional instruction tracing support
* Passes the Klaus Dormann 6502 functional test suite

## Requirements

### Build Tools

* CMake 3.16+
* C compiler with C11 support

### Test Dependencies

The test ROMs are assembled using the CC65 toolchain:

* `ca65`
* `ld65`

Install CC65 from:

https://cc65.github.io/

## Building

Configure the project:

```sh
cmake -S . -B build
```

Build the library:

```sh
cmake --build build
```

The resulting static library is created at:

```text
build/lib6502.a
```

## Running Tests

Build and run all tests:

```sh
ctest --test-dir build --output-on-failure
```

or, from inside the build directory:

```sh
ctest --output-on-failure
```

## Building Test ROMs

Several tests depend on generated 6502 ROM images.

Build all ROMs:

```sh
cmake --build build --target test_roms
```

Generated ROMs are placed in:

```text
build/test/roms/
```

## Using lib6502 in Another CMake Project

If `lib6502` is included as a git submodule:

```cmake
add_subdirectory(external/lib6502)

target_link_libraries(my_emulator PRIVATE 6502)
```

The public include directory is exported automatically.

```c
#include <lib6502/6502.h>
```

## Usage

The CPU does not contain memory internally. Instead, the host application provides callback functions that implement the system bus.

```c
#include <lib6502/6502.h>

uint8_t read(void *ctx, uint16_t addr) {
    return ((uint8_t *)ctx)[addr];
}

void write(void *ctx, uint16_t addr, uint8_t value) {
    ((uint8_t *)ctx)[addr] = value;
}

int main(void) {
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

`lib6502` emulates only the CPU. It does not implement RAM, ROM, graphics, audio, timers, interrupts sources, or peripherals.

A complete system emulator should provide a bus implementation that maps memory accesses to the appropriate hardware components.

Typical uses include:

* NES emulators
* Commodore 64 emulators
* Apple II emulators
* Custom 6502-based systems
* Educational projects and tooling

## License

MIT License.
