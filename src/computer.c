#include "computer.h"
#include <string.h>

void init_computer(Computer *comp) 
{
    memset(comp, 0, sizeof(Computer));
}

void update_computer_state(Computer *cp) 
{
    // ---------------
    // CONTROL & READS
    // ---------------
    
    // FETCH
    wire T[6];
    decode_ring_counter(&cp->ring, T);

    // DECODE
    byte opcode_ir, operand_ir;
    read_instruction_split(&cp->ir, &opcode_ir, &operand_ir);
    DecodedInstruction d_inst = decode_instruction(opcode_ir);

    // Generate signals
    cp->sig = generate_signals(T, d_inst);

    // Get program count
    byte val_PC;
    read_register(&cp->pc.reg, &val_PC);

    // Read from RAM
    byte val_RAM;
    wire ram_addr[3];
    read_MAR(&cp->mar, ram_addr);
    read_ram(&cp->ram, ram_addr, &val_RAM);

    // Read from ACC
    byte val_ACC;
    read_accumulator(&cp->acc, &val_ACC);

    // Compute value in ALU
    byte val_ALU, val_B;
    wire alu_overflow;
    read_B_reg(&cp->b_reg, &val_B);
    val_ALU = ALU(&val_ACC, &val_B, cp->sig.SU, &alu_overflow);

    // ----------------------------------------
    // BUS ARBITRATION (The 8-bit Parallel Bus)
    // ----------------------------------------
    
    // Iterate over the 8 physical bus lines
    for (int i = 0; i < 8; i++) 
    {
        // --- Tri-state Buffers ---
        // Each component has a buffer connecting its bit 'i' to the bus line 'i'.
        
        wire buff_PC  = TRI_STATE(val_PC.bits[i],     cp->sig.CO);
        wire buff_RAM = TRI_STATE(val_RAM.bits[i],    cp->sig.RO);
        wire buff_IR  = TRI_STATE(operand_ir.bits[i], cp->sig.IO);
        wire buff_ACC = TRI_STATE(val_ACC.bits[i],    cp->sig.AO);
        wire buff_ALU = TRI_STATE(val_ALU.bits[i],    cp->sig.EO);

        // --- The Bus Line (Wired-OR) ---
        // In hardware, all buffer outputs touch the same wire. 
        // If any buffer drives '1', the wire is '1'.
        
        wire bus_line = 0; // Default state (Pull-down)
        
        bus_line = OR(bus_line, buff_PC);
        bus_line = OR(bus_line, buff_RAM);
        bus_line = OR(bus_line, buff_IR);
        bus_line = OR(bus_line, buff_ACC);
        bus_line = OR(bus_line, buff_ALU);

        cp->bus.bits[i] = bus_line;
    }

    // -------------------
    // COMPONENTS TRIGGERS
    // -------------------
    
    trigger_PC(&cp->pc, &cp->bus, 0, cp->sig.CE, cp->clk);
    trigger_MAR(&cp->mar, &cp->bus, cp->sig.MI, cp->clk);
    trigger_ram(&cp->ram, ram_addr, &cp->bus, 0, cp->clk);
    trigger_IR(&cp->ir, &cp->bus, cp->sig.II, cp->clk);
    trigger_accumulator(&cp->acc, &cp->bus, cp->sig.AI, cp->clk);
    trigger_B_reg(&cp->b_reg, &cp->bus, cp->sig.BI, cp->clk);
    trigger_out_reg(&cp->out_reg, &cp->bus, cp->sig.OI, cp->clk);

    // Continue executing (increment ring counter) only if not received HLT
    wire run_clk = AND(cp->clk, NOT(cp->sig.HLT));
    trigger_ring_counter(&cp->ring, run_clk, cp->reset);
}
