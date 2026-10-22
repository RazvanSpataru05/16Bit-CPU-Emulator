## CPU Emulator - 16-bit
This application features a 1:1 emulator of the Intel 8086 microprocessor, built in C++ with an interactive ImGui + SFML interface
Designed to simulate a low-level processor including instruction execution, memory access, stack management.

## Features
# Processor
- Full implementation of the Intel 8086 ISA
- 8 general-purpose registers, as well as 4 segment registers
- Flag registers
- Accurate fetch, decode and execute cycle

# Memory
- 20-bit address space (1MB of RAM memory)
- Paginated view featuring 256 pages x 256 addresses each
- Active cell highlighted for PC position
- Read/Write access highlighted cells per instruction cycle
- Free navigation mode

# Execution Control
- Step-by-step execution
- Auto mode: continous execution at configurable speed
- Execution speed slider: 0.1s-2.0s per instruction
- Reset to initial state at any point

# Dissasembler
- Reads machine code from a .txt file
- Translates hexadecimal code back into human-readable assembly mnemonics

# CPU State Viewer
- Real-time display of all registers and flags after each instruction

## Built With
- C++23
- ImGui
- SFML
