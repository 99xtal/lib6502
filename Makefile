CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -Iinclude

SRC = $(wildcard src/*.c)
TEST_SRC = $(wildcard test/*.c)

OBJ_DIR = $(BUILD_DIR)/obj
OBJ = $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SRC))
TEST_BIN = $(TEST_SRC:.c=)

BUILD_DIR = build
LIB = $(BUILD_DIR)/lib6502.a

# Default target
all: $(LIB)

# Build library
$(LIB): $(OBJ)
	mkdir -p $(BUILD_DIR)
	ar rcs $@ $^

# Compile object files

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Build each test executable
test/%: test/%.c $(LIB)
	$(CC) $(CFLAGS) $< $(LIB) -o $@

# Run all tests
test: $(TEST_BIN)
	@echo "Running tests..."
	@for t in $(TEST_BIN); do \
	    echo " → $$t"; \
	    $$t || exit 1; \
	done

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TEST_BIN) src/*.o 
