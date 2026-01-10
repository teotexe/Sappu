#include "memory.h"

void trigger_DFF(DFlipFlop *dff, wire D, wire clk)
{
    // Detect rising edge
    wire not_prev = NOT(dff->prev_clk);
    wire edge = AND(clk, not_prev);

    // If on rising edge change it
    wire newQ = MUX2(dff->Q, D, edge);
    wire newNotQ = NOT(newQ);

    dff->Q = newQ;
    dff->notQ = newNotQ;

    // update prev_clk
    dff->prev_clk = clk;
}

void read_DFF(DFlipFlop *dff, wire *out)
{
    *out = dff->Q;
}

void trigger_register(Register *reg, wire D[REGISTER_SIZE], wire clk) 
{
    for (int i = 0; i < REGISTER_SIZE; i++)
        trigger_DFF(&reg->bits[i], D[i], clk);
}

void read_register(Register *reg, byte *out)
{
    for (int i = 0; i < REGISTER_SIZE; i++)
        read_DFF(&reg->bits[i], &out->bits[i]);   
}

void trigger_PC(ProgramCounter *pc, byte *in, wire load_en, wire count_en, wire clk)
{
    byte curr; 
    read_register(&pc->reg, &curr);
    
    byteAddition increm = ADDER8(&curr, (byte *)&BYTE_ONE, 0);

    byte next_load = MUX2_byte(&curr, &increm.res, count_en);

    byte final_D = MUX2_byte(&next_load, in, load_en);

    trigger_register(&pc->reg, final_D.bits, clk);
}


// ------------- 
//      RAM 
// -------------
void trigger_ram(RAM *ram, wire *addr, byte *data_in, wire write_en, wire clk)
{
    // Decode the address
    // We turn 3 bits (e.g., 010) into 8 lines (0,0,1,0,0,0,0,0)
    wire sel[8];
    decoder_3to8(addr, sel);

    // Update every register in the RAM
    for (int i = 0; i < RAM_SIZE; i++) 
    {    
        // --- CALCULATION OF WRITE ENABLE ---
        // A register should only take new data IF:
        // - It is the selected register (sel[i] is 1)
        // - The write_enable signal is ON
        wire cell_write_en = AND(sel[i], write_en);

        // --- FEEDBACK LOOP (HOLD STATE) ---
        // We need the current value of the register to implement the "Hold"
        byte current_val;
        read_register(&ram->cells[i], &current_val);

        // --- MUX THE INPUT ---
        // If cell_write_en is 1, input is data_in.
        // If cell_write_en is 0, input is current_val (Feedback).
        byte final_input = MUX2_byte(&current_val, data_in, cell_write_en);

        // Trigger the register with the calculated input
        trigger_register(&ram->cells[i], final_input.bits, clk);
    }
}

void read_ram(RAM *ram, wire *addr, byte *data_out)
{
    // Decode address to find which register we care about
    wire sel[8];
    decoder_3to8(addr, sel);

    // Initialize output to 0 (Z-state / Ground)
    for (int i=0; i<8; i++) data_out->bits[i] = 0;

    // "Tri-state" Logic / Multiplexing
    // We look at all registers. We AND their output with their Select Line.
    // Only the selected register will output its actual bits; others output 0.
    // We OR them all together.
    for (int i = 0; i < RAM_SIZE; i++) 
    {    
        byte reg_out;
        read_register(&ram->cells[i], &reg_out);

        // Mask the output: If sel[i] is 0, this register outputs all 0s.
        // If sel[i] is 1, it outputs its data.
        byte masked_out;
        for (int b = 0; b < 8; b++)
            masked_out.bits[b] = AND(reg_out.bits[b], sel[i]);

        // Combine onto the bus
        *data_out = OR_byte(data_out, &masked_out);
    }
}


// -------------
//      MAR
// -------------
void trigger_MAR(MAR *mar, byte *bus_in, wire load_en, wire clk) 
{
    // 1. Feedback: Read current state (to implement "Hold")
    byte current_val;
    read_register(&mar->reg, &current_val);

    // 2. MUX: Select input (Bus if loading, Current if holding)
    byte final_input = MUX2_byte(&current_val, bus_in, load_en);

    // 3. Trigger the internal register
    trigger_register(&mar->reg, final_input.bits, clk);
}

void read_MAR(MAR *mar, wire *addr_out) 
{
    byte val;
    read_register(&mar->reg, &val);
 
    for (int i = 0; i < RAM_SEL_SIZE; i++)
        addr_out[i] = val.bits[i];
}

// --------------------
// INSTRUCTION REGISTER
// --------------------
void trigger_IR(InstructionRegister *ir, byte *bus_in, wire load_en, wire clk)
{
    // 1. Feedback Loop: Get current state
    byte current_val;
    read_register(&ir->reg, &current_val);

    // 2. MUX: Decide if we keep old value or take from Bus
    byte final_input = MUX2_byte(&current_val, bus_in, load_en);

    // 3. Trigger the internal physical register
    trigger_register(&ir->reg, final_input.bits, clk);
}

void read_instruction_split(InstructionRegister *ir, byte *opcode_out, byte *operand_out)
{
    byte val;
    read_register(&ir->reg, &val);

    // SAP-1 Architecture Standard:
    // Bits 0-3 = Operand (The memory address to act on)
    // Bits 4-7 = Opcode  (The operation to perform)

    // Wire the Operand (Lower Nibble)
    for(int i = 0; i < 4; i++) operand_out->bits[i] = val.bits[i];
    for(int i = 4; i < 8; i++) operand_out->bits[i] = 0; // Upper bits grounded

    // Wire the Opcode (Upper Nibble)
    for(int i = 0; i < 4; i++) opcode_out->bits[i] = 0; // Lower bits grounded
    for(int i = 4; i < 8; i++) opcode_out->bits[i] = val.bits[i];
}


void trigger_out_reg(OutputRegister *out_reg, byte *bus_in, wire load_en, wire clk)
{
    byte curr_val;
    read_register(&out_reg->reg, &curr_val);

    byte final_in = MUX2_byte(&curr_val, bus_in, load_en);
    trigger_register(&out_reg->reg, final_in.bits, clk);
}


void read_out_reg(OutputRegister *out_reg, byte *out)
{
    read_register(&out_reg->reg, out);
}







