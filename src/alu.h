#ifndef ALU_H
#define ALU_H

#include "gates.h"
#include "memory.h"

// -----------
// ACCUMULATOR
// -----------
// Stores the running total or current operand
typedef struct {
    Register reg;
} Accumulator;

void trigger_accumulator(Accumulator *acc, byte *bus_in, wire load_en, wire clk);
void read_accumulator(Accumulator *acc, byte *out);

// ----------
// B_Register
// ----------
// Holds the second operand for the arithmetic operation
typedef struct {
    Register reg;
} B_Register;

void trigger_B_reg(B_Register *b_reg, byte *bus_in, wire load_en, wire clk);
void read_B_reg(B_Register *b_reg, byte *out);

// -------------
//      ALU
// -------------
/* A: Operand 1 (from Accumulator)
 * B: Operand 2 (from B_Register)
 * sub_en: 0 = ADD, 1 = SUBTRACT
 * overflow_out: stores carry/overflow
 */
byte ALU(byte *A, byte *B, wire sub_en, wire *overflow_out);

#endif
