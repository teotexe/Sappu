#ifndef CONTROL_H
#define CONTROL_H

#include "memory.h"
#include "gates.h"

// ----------------------
//      RING COUNTER
// ---------------------
// This architecture has 6 time steps (T1-T6). We use a 6-bit ring counter.
// It works by shifting a '1' through a register: 000001, 000010, ..., 100000
typedef struct {
    Register state;
} RingCounter;

void trigger_ring_counter(RingCounter *rc, wire clk, wire reset);

// Helper to get separate wires T1 -> T6
void decode_ring_counter(RingCounter *rc, wire *T);


// ----------------------
//  INSTRUCTION DECODER
// ----------------------
// Decodes the Opcode (upper 4 bits of IR) into active lines.
// LDA: 0000
// ADD: 0001
// SUB: 0010
// OUT: 1110
// HLT: 1111
typedef struct {
    wire is_LDA;
    wire is_ADD;
    wire is_SUB;
    wire is_OUT;
    wire is_HLT;
} DecodedInstruction;

DecodedInstruction decode_instruction(byte opcode);

// -----------------
//  CONTROL MATRIX
// -----------------
typedef struct {
    /* 
     * Fetch Steps (T1-T3)
     * Microcode
     * T1: CO MI (Load PC address into MAR) 
     * T2: RO II (Put instruction at PC address in IR) 
     * T3: CE    (Increment PC)
     */
    // Program Counter Enable signals    
    wire CO; // Output (Put PC value on BUS)
    wire CE; // Incr (Increment PC)

    // RAM Enable signals
    wire MI; // Input (Read Bus into MAR)
    wire RO; // Output (Put RAM data, at MAR address, on Bus)
    
    // IR Input signal
    wire II; // Input (Read Bus into IR)
    
    /*
     * Execute steps (T4-T6)
     * Microcode (LDA)
     * T4: IO MI (Load IR address into MAR)
     * T5: RO AI (Put data at IR address in Accumulator)
     * Microcode (ADD)
     * T4: IO MI (Load IR address into MAR)
     * T5: RO BI (Put data at IR address in B register)
     * T6: EO AI (Put ALU output in the Accumulator)
     * So the instructions
     * LDA 14
     * ADD 15
     * will first load data at RAM address 14 in accumulator
     * ,then load data at RAM address 15 in B register
     * and perform Addition
     */
    // IR output signal
    wire IO; // Output (Put address part of IR on Bus)

    // Accumulator Enable signals    
    wire AI; // Input
    wire AO; // Output (to put on bus result of calculation)

    // B Register Enable signals
    wire BI; // Input
    /** No output since result is stored in the Accumulator **/
    
    // ALU signals
    wire EO; // Output Enable
    wire SU; // Subtract Mode

    // Output register
    wire OI; // Input Enable

    // HALT signal
    wire HLT;
} ControlSignals;

ControlSignals generate_signals(wire *T, DecodedInstruction inst);

#endif
