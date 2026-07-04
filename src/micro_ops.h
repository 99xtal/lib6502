#ifndef MICRO_OPS_H
#define MICRO_OPS_H

typedef struct CPU6502 CPU6502;

typedef void (*MicroOpFn)(CPU6502* cpu);

typedef struct OpDef {
  const char* name;
  MicroOpFn micro_ops[8];
} OpDef;

extern const OpDef reset_sequence;

#endif