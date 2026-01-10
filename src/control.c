#include "control.h"

/* Calculates the next state based on current state
 * Also implements a "Reset" logic: If state is 0, force T1
 */
void trigger_ring_counter(RingCounter *rc, wire clk, wire reset)
{
    byte current;
    read_register(&rc->state, &current);

    // Bit N takes the value of Bit N-1
    // T1 (bit 0) is special: It's 1 only when resetting
    // and makes all other bits 0
    wire T1 = OR(reset, current.bits[5]);
    wire T2 = AND(current.bits[0], NOT(reset));
    wire T3 = AND(current.bits[1], NOT(reset));
    wire T4 = AND(current.bits[2], NOT(reset));
    wire T5 = AND(current.bits[3], NOT(reset));
    wire T6 = AND(current.bits[4], NOT(reset));

    wire next_bits[8] = {T1, T2, T3, T4, T5, T6, 0, 0};

    // Update state
    trigger_register(&rc->state, next_bits, clk);
}

void decode_ring_counter(RingCounter *rc, wire *T)
{
    byte current;
    read_register(&rc->state, &current);
    for (int i = 0; i < 6; i++) T[i] = current.bits[i];
}


DecodedInstruction decode_instruction(byte opcode)
{
    DecodedInstruction inst;

    wire b3 = opcode.bits[7];
    wire b2 = opcode.bits[6];
    wire b1 = opcode.bits[5];
    wire b0 = opcode.bits[4];

    wire n3 = NOT(b3);
    wire n2 = NOT(b2);
    wire n1 = NOT(b1);
    wire n0 = NOT(b0);

    // LDA (0000)
    inst.is_LDA = AND(n3, AND(n2, AND(n1, n0)));

    // ADD (0001)
    inst.is_ADD = AND(n3, AND(n2, AND(n1, b0)));

    // SUB (0010)
    inst.is_SUB = AND(n3, AND(n2, AND(b1, n0)));
    
    // OUT (1110)
    inst.is_OUT = AND(b3, AND(b2, AND(b1, n0)));
    
    // HLT (1111)
    inst.is_HLT = AND(b3, AND(b2, AND(b1, b0)));

    return inst;
}


ControlSignals generate_signals(wire *T, DecodedInstruction inst)
{
    ControlSignals sig = {0};

    // -------------
    //  FETCH CYCLE (T1-T3)
    // -------------
    // Happens regardless of the instruction
    
    // T1: Address State (PC -> MAR)
    // CO (PC Output) = T1
    sig.CO = T[0];

    // MI (MAR Input) = T1 ... OR other later
    wire MI_fetch = T[0];
    
    // T2: Memory State (MAR -> IR)
    // RO (RAM Out) = T3 ... OR other later
    wire RO_fetch = T[1];
    
    // II (IR Input) = T3
    sig.II = T[1];
    
    // T3 : Increment State (PC++)
    sig.CE = T[2];

    // ---------------
    //  EXECUTE CYCLE (T4-T6)
    // ---------------

    // --- LDA (Load Accumulator) ---
    // T4: Address from IR to MAR (IO, MI)
    // T5: Data from RAM to Accum (RO, AI)
    wire LDA_T4 = AND(inst.is_LDA, T[3]);
    wire LDA_T5 = AND(inst.is_LDA, T[4]);

    // --- ADD ---
    // T4: Address from IR to MAR (IO, MI)
    // T5: Data from RAM to B reg (RO, BI)
    // T6: ALU Result to Acc      (EO, AI)
    wire ADD_T4 = AND(inst.is_ADD, T[3]);
    wire ADD_T5 = AND(inst.is_ADD, T[4]);
    wire ADD_T6 = AND(inst.is_ADD, T[5]);
    
    // --- SUB ---
    // Same as ADD, but T6 activates SU (Subtract) flag
    wire SUB_T4 = AND(inst.is_SUB, T[3]);
    wire SUB_T5 = AND(inst.is_SUB, T[4]);
    wire SUB_T6 = AND(inst.is_SUB, T[5]);

    // --- OUT ---
    // T4: Acc to Output Register (AO, OI)
    wire OUT_T4 = AND(inst.is_OUT, T[3]);

    // --- HLT ---
    // Stops the clock
    sig.HLT = inst.is_HLT;

    // -------------------
    // MATRIX AGGREGATION
    // -------------------
    // Combine all conditions for each wire using OR

    // MI: Active on Fetch(T1) OR LDA(T4) OR ADD(T4) OR SUB(T4)
    sig.MI = OR(MI_fetch, OR(LDA_T4, OR(ADD_T4, SUB_T4)));

    // RO: Active on Fetch(T3) OR LDA(T5) OR ADD(T5) OR SUB(T5)
    sig.RO = OR(RO_fetch, OR(LDA_T5, OR(ADD_T5, SUB_T5)));

    // IO: Active on LDA(T4) OR ADD(T4) OR SUB(T4)
    sig.IO = OR(LDA_T4, OR(ADD_T4, SUB_T4));

    // AI: Active on LDA(T5) OR ADD(T6) OR SUB(T6)
    sig.AI = OR(LDA_T5, OR(ADD_T6, SUB_T6));

    // BI: Active on ADD(T5) OR SUB(T5)
    sig.BI = OR(ADD_T5, SUB_T5);

    // EO: Active on ADD(T6) OR SUB(T6)
    sig.EO = OR(ADD_T6, SUB_T6);

    // SU: Active on SUB(T6)
    sig.SU = SUB_T6;

    // AO: Active on OUT(T4)
    sig.AO = OUT_T4;

    // OI: Active on OUT(T4)
    sig.OI = OUT_T4;

    return sig;
}




























