#pragma once

#include "Assembler/OperandKind.h"
#include "Assembler/Slot.h"

#include <array>
#include <string>
#include <unordered_map>
#include <iostream>

namespace ISA
{
	struct ISAEntry
	{
		const char* mnemonic = nullptr;
		uint8_t opcode{};
		OperandKind operatorKind = OperandKind::NONE;
		std::array<Slot, 2> slots = {Slot::NONE, Slot::NONE};
		uint16_t size{};
		bool hasModRM{ false };

		const char* description = nullptr;
	};
	namespace
	{
		const ISAEntry Table[] = {

			/* Load / Store instructions (20 total) */
			{"LDA_IM",  0x01,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },    2, false,  "Loads immediate 8-bit value into register A"},
			{"LDA_DIR", 0x02,   OperandKind::MEM, { Slot::NONE, Slot::NONE },       3, false,  "Loads value at immediate 16-bit memory address into register A"},
			{"LDA_REG", 0x03,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false,  "Loads value into register A"},
			{"STA_DIR", 0x04,   OperandKind::MEM,  { Slot::NONE, Slot::NONE }, 3,   false, "Writes value of register A at 16-bit memory address specified"},
			{"LDB_IM",  0x05,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },   2,  false, "Loads immediate 8-bit value into register B" },
			{"LDB_DIR", 0x06,   OperandKind::MEM, { Slot::NONE, Slot::NONE },  3,  false, "Loads value at immediate 16-bit memory address into register B"},
			{"LDB_REG", 0x07,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },      2, false,  "Loads value from another register into register B"},
			{"STB_DIR", 0x08,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3,  false, "Writes value of register B at 16-bit memory address specified"},
			{"LDC_IM",  0x09,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },   2, false, "Loads immediate 8-bit value into register C" },
			{"LDC_DIR", 0x0A,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3,  false, "Loads value at immediate 16-bit memory address into register C"},
			{"LDC_REG", 0x0B,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false, "Loads value from another register into register C"},
			{"STC_DIR", 0x0C,   OperandKind::MEM, { Slot::NONE, Slot::NONE },  3,  false, "Writes value of register C at 16-bit memory address specified"},
			{"LDD_IM",  0x0D,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },  2,  false, "Loads immediate 8-bit value into register D" },
			{"LDD_DIR", 0x0E,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3,  false, "Loads value at immediate 16-bit memory address into register D"},
			{"LDD_REG", 0x0F,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },    2, false,  "Loads value from another register into register D"},
			{"STD_DIR", 0x10,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3,   false, "Writes value of register D at 16-bit memory address specified"},
			{"LDA_IND", 0x11,   OperandKind::NONE, { Slot::NONE, Slot::NONE },    1, false, "Loads into register A the value from 16-bit register BC"},
			{"STA_IND", 0x12,   OperandKind::NONE, { Slot::NONE, Slot::NONE },    1, false, "Writes into 16-bit memory address of the BC register the value from register A "},
			{"LDW_IM",  0x13,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3,  false, "Loads a word into registers B and C. First 8 bits are loaded into B, following 8 bits are loaded into C"},
			{"INCW",    0x14,   OperandKind::NONE,  { Slot::NONE, Slot::NONE },   1, false, "Forms the 16-bit BC register and increments it"},

			/* Arithmetic operations (12 total) */
			{"ADD_IM",  0x20,   OperandKind::IMM_8,  { Slot::NONE, Slot::NONE },  2, false,  "Adds immediate 8-bit value to register A. Updates flags"},
			{"ADD_REG", 0x21,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false,  "Adds another register value to register A. Updates flags"},
			{"ADC_IM",  0x22,   OperandKind::IMM_8,   { Slot::NONE, Slot::NONE }, 2, false, "Adds immediate value and carry flag to register A. Updates flags"},
			{"ADC_REG", 0x23,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },      2, false,  "Adds another register value to register A and checks for carry flag. Updates flags"},
			{"SUB_IM",  0x24,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },   2,  false, "Subtracts immediate 8-bit value from register A. Updates flags"},
			{"SUB_REG", 0x25,   OperandKind::REG_16,  { Slot::NONE, Slot::NONE },    2, false,  "Subtracts another register value from register A. Updates flags"},
			{"SBC_IM",  0x26,   OperandKind::IMM_8,  { Slot::NONE, Slot::NONE },  2, false, "Subtracts immediate 8-bit value from register A and checks for carry flag. Updates flags"},
			{"SBC_REG", 0x27,   OperandKind::REG_16,  { Slot::NONE, Slot::NONE },    2, false, "Subtracts another register value from register A and checks for carry flag. Updates flags"},
			{"INC",     0x28,   OperandKind::REG_16,  { Slot::NONE, Slot::NONE },    2, false, "Increments specified register by 1. Updates flags"},
			{"DEC",     0x29,   OperandKind::REG_16,   { Slot::NONE, Slot::NONE },   2, false, "Decrements specified register by 1. Updates flags"},
			{"MUL_IM",  0x2A,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },   2, false, "Multiplies register A by immediate 8-bit value. Updates flags"},
			{"MUL_REG", 0x2B,   OperandKind::REG_16,   { Slot::NONE, Slot::NONE },   2, false, "Multiplies another register value by register A. Updates flags"},

			/* Logical operations (9 total) */
			{"AND_IM",  0x30,   OperandKind::IMM_8, { Slot::NONE, Slot::NONE },   2, false, "Applies logical AND between immediate 8-bit value and register A. Updates flags"},
			{"AND_REG", 0x31,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false, "Applies logical AND between specified register and register A. Updates flags"},
			{"OR_IM",   0x32,   OperandKind::IMM_8,  { Slot::NONE, Slot::NONE },  2, false, "Applies logical OR between immediate 8-bit value and register A. Updates flags"},
			{"OR_REG",  0x33,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false, "Applies logical OR between specified register and register A. Updates flags"},
			{"XOR_IM",  0x34,   OperandKind::IMM_8,  { Slot::NONE, Slot::NONE },  2, false, "Applies logical XOR between immediate 8-bit value and register A. Updates flags"},
			{"XOR_REG", 0x35,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false, "Applies logical XOR between specified register and register A. Updates flags"},
			{"NOT",     0x36,   OperandKind::REG_16, { Slot::NONE, Slot::NONE },     2, false, "Applies NOT operator on a specified register. Updates flags"},
			{"SHL",     0x37,   OperandKind::REG_16,  { Slot::NONE, Slot::NONE },    2, false, "Shifts a specified register to the left by 1 bit. Updates flags"},
			{"SHR",     0x38,   OperandKind::REG_16,  { Slot::NONE, Slot::NONE },    2, false,  "Shifts a specified register to the right by 1 bit. Updates flags"},

			/* Compare operations (2 total) */
			{"CMP_IM",  0x40,   OperandKind::IMM_8,  { Slot::NONE, Slot::NONE },  2, false, "Compares immediate 8-bit value to register A. Updates flags"},
			{"CMP_REG", 0x41,   OperandKind::REG_16,   { Slot::NONE, Slot::NONE },   2, false, "Compares specified register value to register A. Updates flags"},

			/* Jump/Branch operations (9 total) */
			{"JMP",     0x50,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC"},
			{"JZ",      0x51,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Zero Flag is true"},
			{"JNZ",     0x52,   OperandKind::MEM,  { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Zero Flag is NOT true"},
			{"JC",      0x53,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Carry Flag is true"},
			{"JNC",     0x54,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Carry Flag is NOT true"},
			{"JN",      0x55,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Negative Flag is true"},
			{"JNN",     0x56,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Negative Flag is NOT true"},
			{"JO",      0x57,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Overflow Flag is true"},
			{"JNO",     0x58,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Computes 16-bit address. Assigns the address to PC if Overflow Flag is NOT true"},

			/* Stack operations (4 total) */
			{"PUSH",    0x60,   OperandKind::REG_16,   { Slot::NONE, Slot::NONE },   2, false, "Pushes the value of a register on the stack"},
			{"POP",     0x61,   OperandKind::REG_16,  { Slot::NONE, Slot::NONE },   2, false, "Pops the value from the top of the stack and assigns it to a register"},
			{"CALL",    0x62,   OperandKind::MEM, { Slot::NONE, Slot::NONE }, 3, false, "Jumps to subroutines at given address, saving return address on the stack"},
			{"RET",     0x63,   OperandKind::NONE, { Slot::NONE, Slot::NONE },     1, false, "Assigns the return address to PC register"},

			/* Misc (6 total) */
			{"NOP",     0x00,   OperandKind::NONE,  { Slot::NONE, Slot::NONE },   1, false, "No operation"},
			{"MOV",     0x70,   OperandKind::REG_REG,{ Slot::NONE, Slot::NONE }, 3, false, "Copies the value of a register to another register"},
			{"EI",      0xF3,   OperandKind::NONE, { Slot::NONE, Slot::NONE },    1, false, "Enables interrupt service routines"},
			{"DI",      0xF8,   OperandKind::NONE,   { Slot::NONE, Slot::NONE },  1, false, "Disables interrupt service routines"},
			{"RETI",    0xF9,   OperandKind::NONE,   { Slot::NONE, Slot::NONE },  1, false, "Assigns to PC the address it had before the interrupt service routine"},
			{"HLT",     0xFF,   OperandKind::NONE,  { Slot::NONE, Slot::NONE },   1, false, "Stops the Program Counter from reading further"}

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

