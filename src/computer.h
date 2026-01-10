#ifndef COMPUTER_H
#define COMPUTER_H

#include <stdio.h>
#include "gates.h"
#include "memory.h"
#include "alu.h"
#include "control.h"

typedef struct {
    // -----------
    // Connections
    // -----------
    byte bus;
    wire clk;
    wire reset;

    // -------------
    // Control logic
    // -------------
    ControlSignals sig;
    RingCounter ring;

    // -----------------
    // Memory components
    // -----------------
    ProgramCounter pc;
    InstructionRegister ir;
    MAR mar;
    RAM ram;

    // --------------
    // ALU components
    // --------------
    Accumulator acc;
    B_Register b_reg;
    OutputRegister out_reg;
} Computer;

void init_computer(Computer *comp);
void update_computer_state(Computer *cp);

void load_program_from_file(Computer *comp, const char *filename);

#endif
