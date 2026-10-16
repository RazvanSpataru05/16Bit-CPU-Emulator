#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <iostream>

namespace ISA
{
    enum class OperatorKind : uint16_t
    {
        NONE,
        IMM_8,
        ADDR_16,
        REG,
        REG_REG
    };

    struct ISAEntry
    {
        const char* mnemonic = nullptr;
        uint16_t opcode{};
        OperatorKind operatorKind{};
        uint16_t size{};
        const char* description = nullptr;
    };

    namespace
    {
        const ISAEntry Table[] = {

            /* Load / Store instructions (20 total) */
            {"LDA_IM",  0x01,   OperatorKind::IMM_8,    2,  "Loads immediate 8-bit value into register A"},
            {"LDA_DIR", 0x02,   OperatorKind::ADDR_16,  3,  "Loads value at immediate 16-bit memory address into register A"},
            {"LDA_REG", 0x03,   OperatorKind::REG,      2,  "Loads value into register A"},
            {"STA_DIR", 0x04,   OperatorKind::ADDR_16,  3,  "Writes value of register A at 16-bit memory address specified"},
            {"LDB_IM",  0x05,   OperatorKind::IMM_8,    2,  "Loads immediate 8-bit value into register B" },
            {"LDB_DIR", 0x06,   OperatorKind::ADDR_16,  3,  "Loads value at immediate 16-bit memory address into register B"},
            {"LDB_REG", 0x07,   OperatorKind::REG,      2,  "Loads value from another register into register B"},
            {"STB_DIR", 0x08,   OperatorKind::ADDR_16,  3,  "Writes value of register B at 16-bit memory address specified"},
            {"LDC_IM",  0x09,   OperatorKind::IMM_8,    2,  "Loads immediate 8-bit value into register C" },
            {"LDC_DIR", 0x0A,   OperatorKind::ADDR_16,  3,  "Loads value at immediate 16-bit memory address into register C"},
            {"LDC_REG", 0x0B,   OperatorKind::REG,      2,  "Loads value from another register into register C"},
            {"STC_DIR", 0x0C,   OperatorKind::ADDR_16,  3,  "Writes value of register C at 16-bit memory address specified"},
            {"LDD_IM",  0x0D,   OperatorKind::IMM_8,    2,  "Loads immediate 8-bit value into register D" },
            {"LDD_DIR", 0x0E,   OperatorKind::ADDR_16,  3,  "Loads value at immediate 16-bit memory address into register D"},
            {"LDD_REG", 0x0F,   OperatorKind::REG,      2,  "Loads value from another register into register D"},
            {"STD_DIR", 0x10,   OperatorKind::ADDR_16,  3,  "Writes value of register D at 16-bit memory address specified"},
            {"LDA_IND", 0x11,   OperatorKind::NONE,     1,  "Loads into register A the value from 16-bit register BC"},
            {"STA_IND", 0x12,   OperatorKind::NONE,     1,  "Writes into 16-bit memory address of the BC register the value from register A "},
            {"LDW_IM",  0x13,   OperatorKind::ADDR_16,  3,  "Loads a word into registers B and C. First 8 bits are loaded into B, following 8 bits are loaded into C"},
            {"INCW",    0x14,   OperatorKind::NONE,     1,  "Forms the 16-bit BC register and increments it"},

            /* Arithmetic operations (12 total) */
            {"ADD_IM",  0x20,   OperatorKind::IMM_8,    2,  "Adds immediate 8-bit value to register A. Updates flags"},
            {"ADD_REG", 0x21,   OperatorKind::REG,      2,  "Adds another register value to register A. Updates flags"},
            {"ADC_IM",  0x22,   OperatorKind::IMM_8,    2,  "Adds immediate value and carry flag to register A. Updates flags"},
            {"ADC_REG", 0x23,   OperatorKind::REG,      2,  "Adds another register value to register A and checks for carry flag. Updates flags"},
            {"SUB_IM",  0x24,   OperatorKind::IMM_8,    2,  "Subtracts immediate 8-bit value from register A. Updates flags"},
            {"SUB_REG", 0x25,   OperatorKind::REG,      2,  "Subtracts another register value from register A. Updates flags"},
            {"SBC_IM",  0x26,   OperatorKind::IMM_8,    2,  "Subtracts immediate 8-bit value from register A and checks for carry flag. Updates flags"},
            {"SBC_REG", 0x27,   OperatorKind::REG,      2,  "Subtracts another register value from register A and checks for carry flag. Updates flags"},
            {"INC",     0x28,   OperatorKind::REG,      2,  "Increments specified register by 1. Updates flags"},
            {"DEC",     0x29,   OperatorKind::REG,      2,  "Decrements specified register by 1. Updates flags"},
            {"MUL_IM",  0x2A,   OperatorKind::IMM_8,    2,  "Multiplies register A by immediate 8-bit value. Updates flags"},
            {"MUL_REG", 0x2B,   OperatorKind::REG,      2,  "Multiplies another register value by register A. Updates flags"},

            /* Logical operations (9 total) */
            {"AND_IM",  0x30,   OperatorKind::IMM_8,    2,  "Applies logical AND between immediate 8-bit value and register A. Updates flags"},
            {"AND_REG", 0x31,   OperatorKind::REG,      2,  "Applies logical AND between specified register and register A. Updates flags"},
            {"OR_IM",   0x32,   OperatorKind::IMM_8,    2,  "Applies logical OR between immediate 8-bit value and register A. Updates flags"},
            {"OR_REG",  0x33,   OperatorKind::REG,      2,  "Applies logical OR between specified register and register A. Updates flags"},
            {"XOR_IM",  0x34,   OperatorKind::IMM_8,    2,  "Applies logical XOR between immediate 8-bit value and register A. Updates flags"},
            {"XOR_REG", 0x35,   OperatorKind::REG,      2,  "Applies logical XOR between specified register and register A. Updates flags"},
            {"NOT",     0x36,   OperatorKind::REG,      2,  "Applies NOT operator on a specified register. Updates flags"},
            {"SHL",     0x37,   OperatorKind::REG,      2,  "Shifts a specified register to the left by 1 bit. Updates flags"},
            {"SHR",     0x38,   OperatorKind::REG,      2,  "Shifts a specified register to the right by 1 bit. Updates flags"},

            /* Compare operations (2 total) */
            {"CMP_IM",  0x40,   OperatorKind::IMM_8,    2, "Compares immediate 8-bit value to register A. Updates flags"},
            {"CMP_REG", 0x41,   OperatorKind::REG,      2, "Compares specified register value to register A. Updates flags"},

            /* Jump/Branch operations (9 total) */
            {"JMP",     0x50,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC"},
            {"JZ",      0x51,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Zero Flag is true"},
            {"JNZ",     0x52,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Zero Flag is NOT true"},
            {"JC",      0x53,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Carry Flag is true"},
            {"JNC",     0x54,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Carry Flag is NOT true"},
            {"JN",      0x55,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Negative Flag is true"},
            {"JNN",     0x56,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Negative Flag is NOT true"},
            {"JO",      0x57,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Overflow Flag is true"},
            {"JNO",     0x58,   OperatorKind::ADDR_16,  3, "Computes 16-bit address. Assigns the address to PC if Overflow Flag is NOT true"},

            /* Stack operations (4 total) */
            {"PUSH",    0x60,   OperatorKind::REG,      2, "Pushes the value of a register on the stack"},
            {"POP",     0x61,   OperatorKind::REG,      2, "Pops the value from the top of the stack and assigns it to a register"},
            {"CALL",    0x62,   OperatorKind::ADDR_16,  3, "Jumps to subroutines at given address, saving return address on the stack"},
            {"RET",     0x63,   OperatorKind::NONE,     1, "Assigns the return address to PC register"},

            /* Misc (6 total) */
            {"NOP",     0x00,   OperatorKind::NONE,     1, "No operation"},
            {"MOV",     0x70,   OperatorKind::REG_REG,  3, "Copies the value of a register to another register"},
            {"EI",      0xF3,   OperatorKind::NONE,     1, "Enables interrupt service routines"},
            {"DI",      0xF8,   OperatorKind::NONE,     1, "Disables interrupt service routines"},
            {"RETI",    0xF9,   OperatorKind::NONE,     1, "Assigns to PC the address it had before the interrupt service routine"},
            {"HLT",     0xFF,   OperatorKind::NONE,     1, "Stops the Program Counter from reading further"}

            /* ISA Table size : 62 total */
        };

        std::unordered_map<std::string, const ISAEntry*> BuildOpcodeTable()
        {
            std::unordered_map<std::string, const ISAEntry*> opcodeTable;
            for (const auto& entry : Table)
            {
                opcodeTable[entry.mnemonic] = &entry;
            }
            return opcodeTable;
        }

        const auto s_opcodeTable = BuildOpcodeTable();
    }

    const ISAEntry* Find(const std::string& mnemonic);
    size_t GetISATableSize();

    bool IsMnemonic(const std::string& word);
}

