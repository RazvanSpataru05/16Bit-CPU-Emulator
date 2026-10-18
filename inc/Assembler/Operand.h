#pragma once

#include "Assembler/OperandKind.h"

struct Operand
{
	ISA::OperandKind kind = ISA::OperandKind::NONE;
	uint8_t			 reg = 0x00;
	uint16_t		 value = 0x00;

	Operand() = default;
	Operand(ISA::OperandKind o, uint8_t r, uint16_t v) :
		kind{ o }, reg{ r }, value{ v } {
	}
};