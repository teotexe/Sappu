#include <stdio.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For system("clear")
#include <string.h> // For strcmp
#include "computer.h" 

// ==========================================
// CONFIGURATION
// ==========================================
// Default speed of the simulation (Hz)
#define DEFAULT_HZ 2

// ==========================================
// HELPERS
// ==========================================

byte int_to_byte(int val) {
    byte out;
    for (int i = 0; i < 8; i++) out.bits[i] = (val >> i) & 1;
    return out;
}

int byte_to_int(byte b) {
    int val = 0;
    for (int i = 0; i < 8; i++) if (b.bits[i]) val |= (1 << i);
    return val;
}

// ==========================================
// VISUALIZATION
// ==========================================

void print_binary(byte b) {
    for(int i=7; i>=0; i--) printf("%d", b.bits[i]);
}

void print_dashboard(Computer *comp) 
{
    // Clear console (ANSI escape)
    printf("\033[H\033[J"); 

    printf("========================================\n");
    printf("      SAPPU GATE-LEVEL SIMULATION       \n");
    printf("========================================\n");
    
    // 1. Clock & Phase
    wire T[6];
    decode_ring_counter(&comp->ring, T);
    int step = 0;
    for(int i=0; i<6; i++) if(T[i]) step = i+1;
    
    printf("CLOCK: [%s]  |  STEP: T%d  |  RESET: %d\n", 
           comp->clk ? "HIGH" : "LOW ", step, comp->reset);
    printf("----------------------------------------\n");

    // 2. Registers
    byte pc, mar, ir, acc, b_reg, out, bus;
    
    read_register(&comp->pc.reg, &pc);
    read_register(&comp->mar.reg, &mar); // In reality MAR output is 4 bits to RAM, but we store 8
    read_register(&comp->ir.reg, &ir);
    read_accumulator(&comp->acc, &acc);
    read_B_reg(&comp->b_reg, &b_reg);
    read_out_reg(&comp->out_reg, &out);
    bus = comp->bus;

    printf(" PC:  %02d   [ ", byte_to_int(pc)); print_binary(pc); printf(" ]\n");
    printf(" MAR: %02d   [ ", byte_to_int(mar)); print_binary(mar); printf(" ]\n");
    printf(" IR:  %02d   [ ", byte_to_int(ir)); print_binary(ir); printf(" ]\n");
    printf(" ACC: %02d   [ ", byte_to_int(acc)); print_binary(acc); printf(" ]\n");
    printf(" B:   %02d   [ ", byte_to_int(b_reg)); print_binary(b_reg); printf(" ]\n");
    printf(" OUT: %02d   [ ", byte_to_int(out)); print_binary(out); printf(" ]\n");
    printf("----------------------------------------\n");
    
    // 3. The Bus (The star of the show)
    printf(" BUS: %02d   [ ", byte_to_int(bus)); print_binary(bus); printf(" ]\n");
    
    // Deduce who is driving the bus for visualization
    // (In hardware, this is physics. Here, we look at the control signals)
    printf(" BUS DRIVER: ");
    if(comp->sig.CO) printf("PC (Program Counter)");
    else if(comp->sig.RO) printf("RAM (Memory)");
    else if(comp->sig.IO) printf("IR (Operand)");
    else if(comp->sig.AO) printf("ACC (Accumulator)");
    else if(comp->sig.EO) printf("ALU (Result)");
    else printf("NONE (Floating/Zero)");
    printf("\n");
    printf("----------------------------------------\n");

    // 4. Control Signals
    printf(" CONTROL WORD:\n");
    printf(" [ %s ] CO (PC Out)\n", comp->sig.CO ? "ON" : "  ");
    printf(" [ %s ] RO (RAM Out)\n", comp->sig.RO ? "ON" : "  ");
    printf(" [ %s ] IO (IR Out)\n", comp->sig.IO ? "ON" : "  ");
    printf(" [ %s ] AO (Acc Out)\n", comp->sig.AO ? "ON" : "  ");
    printf(" [ %s ] EO (ALU Out)\n", comp->sig.EO ? "ON" : "  ");
    printf(" ---\n");
    printf(" [ %s ] MI (MAR In)\n", comp->sig.MI ? "ON" : "  ");
    printf(" [ %s ] II (IR In)\n", comp->sig.II ? "ON" : "  ");
    printf(" [ %s ] AI (Acc In)\n", comp->sig.AI ? "ON" : "  ");
    printf(" [ %s ] BI (B-Reg In)\n", comp->sig.BI ? "ON" : "  ");
    printf(" [ %s ] OI (Out In)\n", comp->sig.OI ? "ON" : "  ");
    printf(" ---\n");
    printf(" [ %s ] CE (Count En)\n", comp->sig.CE ? "ON" : "  ");
    printf(" [ %s ] SU (Subtract)\n", comp->sig.SU ? "ON" : "  ");
    printf(" [ %s ] HLT (Halt)\n", comp->sig.HLT ? "ON" : "  ");

    if (comp->sig.HLT) {
        printf("\n>>> COMPUTER HALTED <<<\n");
    }
}

// ==========================================
// PROGRAM LOADER
// ==========================================
/*
void load_program(Computer *comp) 
{
    printf("Loading Program into RAM...\n");
    
    // PROGRAM: 10 + 4
    // ----------------------------
    // Address 0: LDA 5  (Load data from addr 5)
    // Address 1: ADD 6  (Add data from addr 6)
    // Address 2: OUT    (Output result)
    // Address 3: HLT    (Stop)
    // Address 4: (Empty)
    // Address 5: 10     (Data)
    // Address 6: 4      (Data)

    // Opcodes (Upper 4 bits):
    // LDA: 0000 (0)
    // ADD: 0001 (1)
    // SUB: 0010 (2)
    // OUT: 1110 (14)
    // HLT: 1111 (15)

    // Instructions combined (Opcode << 4 | Address):
    byte ram_data[8];
    ram_data[0] = int_to_byte((0 << 4) | 5);   // LDA 5
    ram_data[1] = int_to_byte((2 << 4) | 6);   // ADD 6
    ram_data[2] = int_to_byte((14 << 4) | 0);  // OUT
    ram_data[3] = int_to_byte((15 << 4) | 0);  // HLT
    ram_data[4] = int_to_byte(0);              // Unused
    ram_data[5] = int_to_byte(10);             // Data: 10
    ram_data[6] = int_to_byte(4);              // Data: 4
    ram_data[7] = int_to_byte(0);              // Unused

    // Manually trigger RAM write for each address
    for(int i=0; i<8; i++) {
        wire addr[3];
        addr[0] = (i >> 0) & 1;
        addr[1] = (i >> 1) & 1;
        addr[2] = (i >> 2) & 1;

        // Pulse the clock on the RAM to write data
        // Setup (Clk Low) -> Trigger (Clk High) -> Hold (Clk Low)
        trigger_ram(&comp->ram, addr, &ram_data[i], 1, 0); 
        trigger_ram(&comp->ram, addr, &ram_data[i], 1, 1);
        trigger_ram(&comp->ram, addr, &ram_data[i], 0, 0); // Disable write
    }
}
*/

void load_program(Computer *comp, const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    // SAP-1 has 8 bytes of RAM.
    unsigned char file_buffer[8] = {0};

    // Read the file into our C buffer
    size_t bytes_read = fread(file_buffer, 1, 8, file);
    fclose(file);

    // Loop through the bytes we read (up to 16)
    for(size_t i = 0; i < bytes_read; i++) 
    {
        // 1. Convert standard C byte to your struct 'byte'
        byte data_to_write = int_to_byte(file_buffer[i]);

        // 2. Setup Address Wires
        // We need 3 bits to address 8 bytes (000 to 111)
        wire addr[3];
        addr[0] = (i >> 0) & 1; // LSB
        addr[1] = (i >> 1) & 1;
        addr[2] = (i >> 2) & 1; // MSB

        // 3. Pulse the clock on the RAM to write data
        // Sequence: Setup Address/Data -> Enable Write -> Clock High -> Clock Low -> Disable Write

        // Step A: Set Address and Data, Write Enable = 1, Clock = 0
        trigger_ram(&comp->ram, addr, &data_to_write, 1, 0);

        // Step B: Clock High (Trigger the write)
        trigger_ram(&comp->ram, addr, &data_to_write, 1, 1);

        // Step C: Write Enable = 0, Clock = 0 (Safe state)
        trigger_ram(&comp->ram, addr, &data_to_write, 0, 0);
    }

}

// ==========================================
// MAIN
// ==========================================
void print_usage(const char *prog) {
    printf("Usage: %s <program.bin> [options]\n", prog);
    printf("Options:\n");
    printf("  -auto [hz]  Auto mode with optional clock speed (default: %d Hz)\n", DEFAULT_HZ);
    printf("  -man        Manual mode (default)\n");
    printf("\nExamples:\n");
    printf("  %s ram.bin              # Manual mode\n", prog);
    printf("  %s ram.bin -auto        # Auto mode at %d Hz\n", prog, DEFAULT_HZ);
    printf("  %s -auto 5 ram.bin      # Auto mode at 5 Hz\n", prog);
    printf("  %s -man ram.bin         # Manual mode\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse arguments
    int auto_mode = 0;
    int clock_hz = DEFAULT_HZ;

    if (argc >= 3) 
    {
        if (strcmp(argv[2], "-auto") == 0) {
            auto_mode = 1;
        } else if (strcmp(argv[2], "-man") != 0) {
            printf("Error: Unknown mode '%s'\n", argv[2]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (argc >= 4 && auto_mode) {
        clock_hz = atoi(argv[3]);
        if (clock_hz <= 0) clock_hz = DEFAULT_HZ;
    }

    int half_period_us = (1000000 / clock_hz) / 2;

    Computer comp;
    init_computer(&comp);
    
    printf("\033[H\033[J"); 

    // Print banner and wait
    printf("\n");
    printf("██████   ██████   ██████   ██████   ██  ██    \n");
    printf("██       ██  ██   ██  ██   ██  ██   ██  ██   \n");
    printf("██████   ██████   ██████   ██████   ██  ██   \n");
    printf("    ██   ██  ██   ██       ██       ██  ██   \n");
    printf("██████   ██  ██   ██       ██       ██████\n");
    printf("\n");
    printf("Press Enter to load program into RAM...\n");
    getchar();

    // 1. Load the Code
    load_program(&comp, argv[1]);

    // Render Display
    print_dashboard(&comp);
    printf("Mode: %s", auto_mode ? "AUTO" : "MANUAL");
    if (auto_mode) printf(" (%d Hz)", clock_hz);
    printf("\nPress Enter to start...\n");
    getchar();

    // 2. Reset Sequence
    // We must toggle the reset pin while the clock runs briefly 
    // to force the Ring Counter to T1 (000001)
    comp.clk = 0; comp.reset = 1; update_computer_state(&comp);
    comp.clk = 1; comp.reset = 1; update_computer_state(&comp);
    comp.clk = 0; comp.reset = 0; update_computer_state(&comp);
    
    // 3. Main Loop
    while (1) {
        // --- CLOCK LOW ---
        comp.clk = 0;
        update_computer_state(&comp);
        
        // Render Display
        print_dashboard(&comp);

        if (auto_mode) {
            usleep(half_period_us);
        } else {
            getchar();
        }

        // --- CLOCK HIGH ---
        // This is where triggers happen (Rising Edge)
        comp.clk = 1;
        update_computer_state(&comp);

        if (auto_mode) {
            usleep(half_period_us);
        }
        
        // Check Halt
        if (comp.sig.HLT) {
            print_dashboard(&comp);
            break;
        }
    }

    return 0;
}
