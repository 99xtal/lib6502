CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude -Isrc

BUILD_DIR = build

LIB = $(BUILD_DIR)/lib6502.a

# Source files
SRC = $(wildcard src/*.c)
TEST_SRC = $(wildcard test/*.c)
ROM_SRC = $(wildcard test/roms/*.asm)

# Build directories
OBJ_DIR = $(BUILD_DIR)/obj
TEST_DIR = $(BUILD_DIR)/test
ROM_DIR = $(TEST_DIR)/roms

# Generated files
OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
TEST_BIN = $(patsubst test/%.c,$(TEST_DIR)/%,$(TEST_SRC))
ROM_OBJ = $(patsubst test/roms/%.asm,$(ROM_DIR)/%.o,$(ROM_SRC))
ROM_BIN = $(ROM_OBJ:.o=.bin)

# Default target
all: $(LIB)

# Build static library
$(LIB): $(OBJ)
	mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

# Compile library objects
$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test executables
$(TEST_DIR)/%: test/%.c $(LIB)
	mkdir -p $(TEST_DIR)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

# Assemble 6502 test ROM object files
$(ROM_DIR)/%.o: test/roms/%.asm
	mkdir -p $(ROM_DIR)
	ca65 $< -o $@

# Link ROM images
$(ROM_DIR)/%.bin: $(ROM_DIR)/%.o test/roms/test-machine.cfg
	mkdir -p $(ROM_DIR)
	ld65 $< -C test/roms/test-machine.cfg -o $@

# Build ROMs and run C tests
test: $(TEST_BIN) $(ROM_BIN)
	@echo "Running tests..."
	@for t in $(TEST_BIN); do \
		echo " → $$t"; \
		$$t || exit 1; \
	done

# Debug helper
print:
	@echo "SRC      = $(SRC)"
	@echo "TEST_SRC = $(TEST_SRC)"
	@echo "ROM_SRC  = $(ROM_SRC)"
	@echo "OBJ      = $(OBJ)"
	@echo "TEST_BIN = $(TEST_BIN)"
	@echo "ROM_OBJ  = $(ROM_OBJ)"
	@echo "ROM_BIN  = $(ROM_BIN)"

# Remove all generated artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean print