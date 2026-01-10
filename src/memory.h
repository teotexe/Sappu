#ifndef MEMORY_H
#define MEMORY_H

#include "gates.h"

#define REGISTER_SIZE 8
#define RAM_SIZE 8
#define RAM_SEL_SIZE 3  // Size of the MUX to decode address 2^3 = 8

typedef struct {
    wire Q;
    wire notQ;
    wire prev_clk;
} DFlipFlop;

void trigger_DFF(DFlipFlop *dff, wire D, wire clk);

typedef struct {
    DFlipFlop bits[REGISTER_SIZE];
} Register;

void trigger_register(Register *reg, wire D[REGISTER_SIZE], wire clk);
void read_register(Register *reg, byte *out);

typedef struct {
    Register reg;
} ProgramCounter;

void trigger_PC(ProgramCounter *pc, byte *in, wire load_en, wire count_en, wire clk);


// --------------
//      RAM 
// --------------
typedef struct {
    Register cells[RAM_SIZE];
} RAM;

/* TRIGGER
 * addr: 3-bit wire array selecting the address
 * data_in: The byte to write
 * write_en: 1 = Write to address, 0 = Read only (Registers hold value)
 * clk: The clock signal
 */
void trigger_ram(RAM *ram, wire *addr, byte *data_in, wire write_en, wire clk);

/* READ
 * addr: 3-bit wire array selecting the address
 * data_out: Pointer to store the read byte
 */
void read_ram(RAM *ram, wire *addr, byte *data_out);


// -----------------------
// MEMORY ADDRESS REGISTER
// -----------------------
// It listens to the BUS, but outputs to the RAM's address lines (not the bus)
typedef struct {
    Register reg;
} MAR;

/* TRIGGER
 * bus_in: The 8-bit data bus
 * load_en: Control signal to load data
 * clk: clock signal
 */
void trigger_MAR(MAR *mar, byte *bus_in, wire load_en, wire clk);

/* READ
 * addr_out: Stores the read address
 */
void read_MAR(MAR *mar, wire *addr_out);

// -------------------- 
// INSTRUCTION REGISTER 
// --------------------
typedef struct {
    Register reg;
} InstructionRegister;

/* TRIGGER
 * bus_in: The 8-bit data bus
 * load_en: Control signal to load data from bus into IR
 * clk: Clock signal
 */
void trigger_IR(InstructionRegister *ir, byte *bus_in, wire load_en, wire clk);

/* READ
 * opcode_out: Receives upper 4 bits (MSB)  0000XXXX
 * operand_out: Receives lower 4 bits (LSB) XXXX0000
 */
void read_instruction_split(InstructionRegister *ir, byte *opcode_out, byte *operand_out);

// -------------------------
//      OUTPUT REGISTER
// -------------------------
// Stores the output of the programs
typedef struct {
    Register reg;
} OutputRegister;

/* TRIGGER
 * bus_in: The 8-bit data bus
 * load_en: Control signal to load data
 * clk: clock signal
 */
void trigger_out_reg(OutputRegister *out_reg, byte *bus_in, wire load_en, wire clk);

/* READ
 * out: Stores the read data
 */
void read_out_reg(OutputRegister *out_reg, byte *out);


#endif
