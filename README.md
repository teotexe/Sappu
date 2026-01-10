<div style="text-align: center;">
<pre style="font-family: monospace; display: inline-block; text-align: left;">
███████╗  ██████╗  ██████╗  ██████╗  ██╗   ██╗
██╔════╝ ██╔═══██╗ ██╔══██╗ ██╔══██╗ ██║   ██║
███████╗ ████████║ ██████╔╝ ██████╔╝ ██║   ██║
╚════██║ ██╔═══██║ ██╔═══╝  ██╔═══╝  ██║   ██║
███████║ ██║   ██║ ██║      ██║      ╚██████╔╝
╚══════╝ ╚═╝   ╚═╝ ╚═╝      ╚═╝       ╚═════╝
</pre>
</div>

<h1 align="center">Simple As Possible Processing Unit</h1>

<h3 align="center"><i>A gate-level digital logic simulator of an 8-bit CPU architecture.</i></h3>

<p align="center"><b>No high-level emulation.</b> Every wire, gate, flip-flop, and bus line is explicitly modeled to simulate real hardware behavior.</p>

![Example](assets/example.gif)

## Why I built this

I built Sappu to bridge the gap between high-level programming and physical hardware. 

Many emulators simulate behavior (e.g., `acc += val`). Sappu simulates the **electricity**. The entire CPU is built on top of just two C primitives: `NAND` and `NOT`. From there, I implemented gates, then components, then the bus.

It serves as a **readable reference implementation** for anyone who wants to see how:
- **C Pointers** can model hardware buses.
- **Bitwise operations** create logic gates.
- **Control Signals** physically drive the fetch-execute cycle.

## Index

- [Architecture](#architecture)
- [Instruction Set](#instruction-set)
- [Writing Programs](#writing-programs)
- [Building & Running](#building--running)
- [Project Structure](#project-structure)
- [Microcode](#microcode-fetch-execute-cycle)
- [Control Signals](#control-signals-reference)

## Architecture

The architecture is based on the SAP-1 (Simple As Possible) computer.

![Example](assets/scheme.png)

### Components

| Component | Description |
|-----------|-------------|
| **PC** | Program Counter - holds address of next instruction |
| **MAR** | Memory Address Register - selects RAM address |
| **RAM** | 8 bytes of memory (addresses 0-7) |
| **IR** | Instruction Register - holds current instruction |
| **ACC** | Accumulator - main working register |
| **B** | B Register - second operand for ALU |
| **ALU** | Arithmetic Logic Unit - performs add/subtract |
| **OUT** | Output Register - displays results |
| **Control Unit** | Generates control signals based on instruction |

## Instruction Set

| Mnemonic | Opcode | Format | Description |
|----------|--------|--------|-------------|
| `LDA` | `0000` | `LDA addr` | Load value from RAM[addr] into Accumulator |
| `ADD` | `0001` | `ADD addr` | Add RAM[addr] to Accumulator |
| `SUB` | `0010` | `SUB addr` | Subtract RAM[addr] from Accumulator |
| `OUT` | `1110` | `OUT` | Output Accumulator value to display |
| `HLT` | `1111` | `HLT` | Halt the CPU |

## Writing Programs

RAM is **8 bytes** (addresses 0-7). Each line in your assembly file corresponds to a RAM address. Use `DAT` to store data values.

```asm
# Address | Instruction
# --------|------------
# 0       | LDA 5       # Load 10 from address 5
# 1       | ADD 6       # Add 4 from address 6
# 2       | OUT         # Output result (14)
# 3       | HLT         # Stop
# 4       |             # Unused padding
# 5       | DAT 10      # First number (10)
# 6       | DAT 4       # Second number (4)

```

## Building & Running

```bash
# Build
make

# Quick start (auto-run mode)
./sap src/program.asm -auto

# Manual stepping (useful for debugging)
./assembler src/program.asm
./run ram.bin

```

## Project Structure

I've organized the code to separate the abstraction layers:

```
src/
├── gates.c       # The physics: AND, OR, NOT, XOR implementation
├── alu.c         # The math: Adder/Subtractor built from gates
├── memory.c      # The storage: Flip-flops and Registers
├── control.c     # The brain: Signal generation
├── computer.c    # The motherboard: Wiring it all together
└── assembler.c   # The tool: Assembly to binary converter

```

## Microcode (Fetch-Execute Cycle)

### Fetch Phase

| Step | Signals | Action |
| --- | --- | --- |
| T1 | `CO MI` | PC → Bus → MAR |
| T2 | `RO II` | RAM[MAR] → Bus → IR |
| T3 | `CE` | PC++ |

### Execute Phase (Example: ADD)

| Step | Signals | Action |
| --- | --- | --- |
| T4 | `IO MI` | IR[3:0] → Bus → MAR |
| T5 | `RO BI` | RAM[MAR] → Bus → B |
| T6 | `EO AI` | ALU(ACC+B) → Bus → ACC |

## Control Signals Reference

| Signal | Name | Description |
| --- | --- | --- |
| `CO` | Counter Out | PC outputs to bus |
| `CE` | Counter Enable | Increment PC |
| `MI` | MAR In | Load MAR from bus |
| `RO` | RAM Out | RAM outputs to bus |
| `II` | IR In | Load IR from bus |
| `IO` | IR Out | IR operand to bus |
| `AI` | Accumulator In | Load ACC from bus |
| `AO` | Accumulator Out | ACC outputs to bus |
| `BI` | B Register In | Load B from bus |
| `EO` | ALU Out | ALU outputs to bus |
| `SU` | Subtract | ALU subtract mode |
| `OI` | Output In | Load OUT from bus |
| `HLT` | Halt | Stop execution |

## Learning Resources

Inspired by:

* **"Digital Computer Electronics"** by Malvino & Brown (The SAP-1 architecture)
* **Ben Eater** (8-bit breadboard computer series)

## License

MIT License. Copyright (c) 2026 Matteo Tacconi.
