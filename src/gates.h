#ifndef GATES_H
#define GATES_H

typedef int wire;

typedef struct {
    wire sum;
    wire carry;
} Addition;

wire NOT(wire a);
wire NAND(wire a, wire b);
wire AND(wire a, wire b);
wire OR(wire a, wire b);
wire XOR(wire a, wire b);

Addition ADDER(wire a, wire b, wire carryIn);
wire MUX2(wire a, wire b, wire sel);

typedef struct {
    wire bits[8];
} byte;

static const byte BYTE_ONE = { .bits = { 1,0,0,0,0,0,0,0 } };

typedef struct {
    byte res;
    wire overflow;
} byteAddition;

byte NOT_byte(byte *a);
byte NAND_byte(byte *a, byte *b);
byte AND_byte(byte *a, byte *b);
byte OR_byte(byte *a, byte *b); 
byte XOR_byte(byte *a, byte *b);

byteAddition ADDER8(byte *a, byte *b, wire carryIn);
byte MUX2_byte(byte *a, byte *b, wire sel);

void decoder_3to8(wire *addr, wire *out);

/* * TRI_STATE BUFFER
 * Real hardware:
 * If enable=1, Out = In
 * If enable=0, Out = High Impedance (Z)
 */
wire TRI_STATE(wire in, wire enable);

#endif


