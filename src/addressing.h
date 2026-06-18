#ifndef ADDRESSING_H
#define ADDRESSING_H

#include <lib6502/6502.h>
#include "opcodes.h"

Operand addr_imm(cpu6502 *cpu);

Operand addr_abs(cpu6502 *cpu);

Operand addr_abs_x(cpu6502 *cpu);

Operand addr_abs_y(cpu6502 *cpu);

Operand addr_indirect(cpu6502 *cpu);

Operand addr_indirect_x(cpu6502 *cpu);

Operand addr_indirect_y(cpu6502 *cpu);

Operand addr_zero_page(cpu6502 *cpu);

Operand addr_zero_page_x(cpu6502 *cpu);

Operand addr_zero_page_y(cpu6502 *cpu);

Operand addr_implied(cpu6502 *cpu);

Operand addr_rel(cpu6502 *cpu);

Operand addr_acc(cpu6502 *cpu);

#endif // ADDRESSING_H