#include "alu.h"

void trigger_accumulator(Accumulator *acc, byte *bus_in, wire load_en, wire clk)
{
    byte curr_val;
    read_register(&acc->reg, &curr_val);

    byte final_in = MUX2_byte(&curr_val, bus_in, load_en);
    trigger_register(&acc->reg, final_in.bits, clk);
}

void read_accumulator(Accumulator *acc, byte *out)
{
    read_register(&acc->reg, out);
}


void trigger_B_reg(B_Register *b_reg, byte *bus_in, wire load_en, wire clk)
{
    byte curr_val;
    read_register(&b_reg->reg, &curr_val);

    byte final_in = MUX2_byte(&curr_val, bus_in, load_en);
    trigger_register(&b_reg->reg, final_in.bits, clk);
}

void read_B_reg(B_Register *b_reg, byte *out)
{
    read_register(&b_reg->reg, out);
}

byte ALU(byte *A, byte *B, wire sub_en, wire *overflow_out)
{
    // Control mask for the XOR
    byte sub_mask;
    for (int i = 0; i < 8; i++) sub_mask.bits[i] = sub_en;

    /* CONDITIONAL INVERTER for B
     * If sub_en = 1 --> B = NOT(B)
     * else B = B
     */
    byte B_processed;
    B_processed = XOR_byte(B, &sub_mask);

    /* ADDITION
     * If subtracting we add sub_en as carry_in
     * so we get A + ~B + 1 (Two's complement subtraction)
     */
    byteAddition result = ADDER8(A, &B_processed, sub_en);

    *overflow_out = result.overflow;
    return result.res;
}

