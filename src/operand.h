#ifndef OPERAND_H
#define OPERAND_H

#include <stdint.h>

typedef enum {
    OPERAND_MEMORY,
    OPERAND_ACCUMULATOR,
} OperandType;

typedef struct {
    OperandType type;
    uint16_t addr;
    int page_crossed;
} Operand;

#endif