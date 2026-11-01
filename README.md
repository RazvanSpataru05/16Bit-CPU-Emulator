# CPU Emulator - Intel 8086

An emulator of the Intel 8086 microprocessor (1978), built in C++ with an interactive ImGui + SFML interface.
It simulates a low-level processor, including instruction execution, memory access and stack management.

> **Work in progress:** the 8086 instruction set is only partially implemented. See [Instruction set status](#instruction-set-status).

## Features

### Processor
- 16-bit registers: `AX`, `BX`, `CX`, `DX` (each with 8-bit halves, e.g. `AH`/`AL`), `SP`, `BP`, `SI`, `DI`
- Segment registers: `CS`, `DS`, `ES`, `SS`
- `FLAGS` register with the status flags (`CF`, `PF`, `AF`, `ZF`, `SF`, `OF`) and control flags
- Fetch, decode and execute cycle, with `segment:offset` addressing (`CS:IP`) and little-endian words

### Memory
- 20-bit address space (1 MB), byte-addressable
- Paginated view of the memory
- Active cell highlighted for the instruction pointer
- Read/write access highlighted per instruction cycle
- Free navigation mode

### Execution control
- Step-by-step execution
- Auto mode: continuous execution at a configurable speed
- Execution speed slider: 0.1s - 2.0s per instruction
- Reset to the initial state at any point

### CPU state viewer
- Real-time display of all registers and flags after each instruction
- Loads machine code from a `.txt` file

## Instruction set status

### Implemented
- **Data movement:** `MOV` register, immediate; `MOV` register, register; `XCHG` AX, register; `CBW`, `CWD`
- **Arithmetic and logic:** `ADD`, `ADC`, `SUB`, `SBB`, `CMP`, `AND`, `OR`, `XOR` (accumulator-immediate and register-register forms); `INC`, `DEC`; `DAA`, `DAS`, `AAA`, `AAS`
- **Control flow:** `JMP` (short, near, far, register-indirect); all conditional jumps; `LOOP`, `LOOPZ`, `LOOPNZ`, `JCXZ`; `CALL` (near, far, register-indirect); `RETF`
- **Stack:** `PUSH` and `POP` for 16-bit registers and segment registers
- **Flags:** `CLC`, `STC`, `CMC`, `CLI`, `STI`, `CLD`, `STD`
- **Other:** `NOP`, `HLT`, `WAIT`

### Not implemented yet
- ModR/M addressing with memory operands (currently only register-register)
- Immediate groups (`0x80`-`0x83`), shifts and rotates, `MUL`/`DIV`/`NOT`/`NEG`/`TEST`
- String instructions and the `REP` prefixes, `MOV` with a direct memory address
- Segment override prefixes
- `INT` / `IRET` and interrupt handling
- `IN` / `OUT`
- 8086 assembler and disassembler (being migrated from the previous custom 8-bit ISA)

## Built with
- C++23
- ImGui
- SFML
