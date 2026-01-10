#include "gates.h"

/*
        NAND AND OR  XOR
    0 0   1   0   0   0              
    0 1   1   0   1   1          
    1 0   1   0   1   1          
    1 1   0   1   1   0          
*/

wire NOT(wire a)
{
    return !a;
}

wire NAND(wire a, wire b)
{
   return !(a && b);
}

wire AND(wire a, wire b)
{
    wire n = NAND(a, b);
    return NAND(n, n);
}

wire OR(wire a, wire b)
{
    wire na = NAND(a, a);
    wire nb = NAND(b, b);
    return NAND(na, nb);
}

wire XOR(wire a, wire b)
{
    wire n = NAND(NAND(a, b), OR(a, b));
    return NAND(n, n);
}



byte NOT_byte(byte *a)
{
    byte res;
    
    for (int i = 0; i < 8; i++)
        res.bits[i] = NOT(a->bits[i]);

    return res;

}

byte NAND_byte(byte *a, byte *b)
{
    byte res;
    
    for (int i = 0; i < 8; i++)
        res.bits[i] = NAND(a->bits[i], b->bits[i]);

    return res;

}

byte AND_byte(byte *a, byte *b)
{
    byte res;
    
    for (int i = 0; i < 8; i++)
        res.bits[i] = AND(a->bits[i], b->bits[i]);

    return res;

}

byte OR_byte(byte *a, byte *b) 
{
    byte res;
    
    for (int i = 0; i < 8; i++)
        res.bits[i] = OR(a->bits[i], b->bits[i]);

    return res;
}


byte XOR_byte(byte *a, byte *b)
{
    byte res;
    
    for (int i = 0; i < 8; i++)
        res.bits[i] = XOR(a->bits[i], b->bits[i]);

    return res;

}

Addition ADDER(wire a, wire b, wire carryIn)
{
    Addition res;

    wire xor = XOR(a, b);

    res.sum = XOR(xor, carryIn);
    
    wire gen = AND(a, b);
    wire prop = AND(carryIn, xor);

    res.carry = OR(gen, prop);

    return res;
}

byteAddition ADDER8(byte *a, byte *b, wire carryIn)
{
    byteAddition res;

    wire carryCurr = carryIn;

    for (int i = 0; i < 8; i++)
    {
        Addition step = ADDER(a->bits[i], b->bits[i], carryCurr);

        res.res.bits[i] = step.sum;

        carryCurr = step.carry;
    }

    res.overflow = carryCurr;

    return res;
}



// Multiplexer to select a bit (a or b)
wire MUX2(wire a, wire b, wire sel)
{
    wire not_sel = NOT(sel);

    wire pass_a = AND(a, not_sel);
    wire pass_b = AND(b, sel);

    return OR(pass_a, pass_b);
}

// Multiplexer to select a byte
byte MUX2_byte(byte *a, byte *b, wire sel)
{
    byte res;

    for (int i = 0; i < 8; i++)
        res.bits[i] = MUX2(a->bits[i], b->bits[i], sel);

    return res;
}


// A 3-to-8 Decoder
// addr: array of 3 wires (addr[0] is LSB)
// out: array of 8 wires (one-hot output)
void decoder_3to8(wire *addr, wire *out)
{
    wire notA0 = NOT(addr[0]);
    wire notA1 = NOT(addr[1]);
    wire notA2 = NOT(addr[2]);
                
    out[0] = AND(notA2, AND(notA1, notA0));	    // 000
    out[1] = AND(notA2, AND(notA1, addr[0]));	    // 001
    out[2] = AND(notA2, AND(addr[1], notA0));	    // 010
    out[3] = AND(notA2, AND(addr[1], addr[0]));	    // 011
    out[4] = AND(addr[2], AND(notA1, notA0)); 	    // 100
    out[5] = AND(addr[2], AND(notA1, addr[0]));	    // 101
    out[6] = AND(addr[2], AND(addr[1], notA0));	    // 110
    out[7] = AND(addr[2], AND(addr[1], addr[0]));   // 111
}


wire TRI_STATE(wire in, wire enable)
{
    // Functionally identical to AND in this simulation model,
    // but semantically represents a switch.
    return AND(in, enable);
}
