#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// SAP-1 Instruction Set
// Maps string "LDA" to integer 0 (0000)
int get_opcode(char *instr) 
{
    // Convert to uppercase for safety
    for(int i = 0; instr[i]; i++) instr[i] = toupper(instr[i]);

    if (strcmp(instr, "LDA") == 0) return 0;  // 0000
    if (strcmp(instr, "ADD") == 0) return 1;  // 0001
    if (strcmp(instr, "SUB") == 0) return 2;  // 0010
    
    // ... Future instructions go here ...

    if (strcmp(instr, "OUT") == 0) return 14; // 1110
    if (strcmp(instr, "HLT") == 0) return 15; // 1111
    
    return -1; // Error
}

int main(int argc, char **argv) 
{
    if (argc < 2) 
    {
        printf("Usage: ./assembler program.asm\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    FILE *out = fopen("ram.bin", "wb"); // Write Binary
    
    if (!in) { printf("Error: Could not open source file.\n"); return 1; }

    char line[100];
    int line_num = 0;
    
    // We have 8 bytes of RAM. Initialize with 0.
    unsigned char memory[8] = {0}; 
    int program_counter = 0;

    printf("\033[H\033[J"); 
    printf("--- ASSEMBLING %s ---\n", argv[1]);

    while (fgets(line, sizeof(line), in)) 
    {
        line_num++;
        
        // 1. Clean up line (Remove comments, trim spaces)
        char *token = strtok(line, " \t\n\r");
        
        // Skip empty lines or comments
        if (token == NULL || token[0] == '/' || token[0] == '#') continue;

        // 2. Check if it is a DATA definition (e.g. "DAT 10")
        // Not standard SAP-1, but useful for variable declaration at specific lines
        if (strcasecmp(token, "DAT") == 0) 
        {
            char *val_str = strtok(NULL, " \t\n\r");
            if (val_str) 
            {
                int val = atoi(val_str);
                memory[program_counter++] = (unsigned char)val;
                printf("[%02d] DATA: %d\n", program_counter-1, val);
            }
            continue;
        }

        // 3. Parse Instruction
        int opcode = get_opcode(token);
        if (opcode == -1) 
        {
            printf("Error Line %d: Unknown Instruction '%s'\n", line_num, token);
            continue;
        }

        // 4. Parse Operand (The number after the instruction)
        // Some instructions (OUT, HLT) don't need operands, but SAP-1 format 
        // usually includes dummy bits. We parse if available.
        int operand = 0;
        char *op_str = strtok(NULL, " \t\n\r");
        
        if (op_str != NULL)
            operand = atoi(op_str);

        // 5. Combine: [Opcode 4 bits] [Operand 4 bits]
        unsigned char byte = (opcode << 4) | (operand & 0x0F);
        
        memory[program_counter] = byte;
        printf("[%02d] %s %d \t-> Hex: %02X\n", program_counter, token, operand, byte);
        
        program_counter++;
        if (program_counter >= 8) 
        {
            printf("Error: Program too large for 8-byte RAM.\n");
            break;
        }
    }

    // Write the 16 bytes to file
    fwrite(memory, 1, 8, out);
    
    fclose(in);
    fclose(out);
    printf("--- SUCCESS: ram.bin created ---\n");
    return 0;
}
