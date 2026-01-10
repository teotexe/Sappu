#include <stdio.h>

int main(int argc, char **argv)
{
     if (argc < 2) 
    {
        printf("Usage: ./linker program.bin\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "rb");
    
    if (in == NULL)
    {
        printf("Error opening file...\n");
        return 1;
    }

    unsigned char ram[16];
    size_t bytes_read = fread(ram, 1, 16, in);

    fclose(in);
    
    printf("--- Reading RAM Image ---\n");
    for (int i = 0; i < 16; i++) 
    {
        // Print index, Hex value, and Binary breakdown (Opcode/Operand)
        unsigned char opcode = (ram[i] >> 4) & 0x0F;
        unsigned char operand = ram[i] & 0x0F;
        
        printf("Address [%02d]: Hex: 0x%02X (Opcode: %d, Operand: %d)\n", i, ram[i], opcode, operand);
    }

    return 0;
}
