#pragma once

#include "Core/Result.h"

#include "Utils/Constants.h"

#include <bit>

namespace ALU
{
	Result Execute(uint8_t operation, uint16_t a, uint16_t b, bool word, bool carryIn);
	Result Shift(uint8_t operation, uint16_t value, uint8_t count, bool word, bool carryIn);

	Result ADD(uint16_t a, uint16_t b, bool carryIn, bool word);
	Result SUB(uint16_t a, uint16_t b, bool borrowIn, bool word);

	Result INC(uint16_t a, bool word);
	Result DEC(uint16_t a, bool word);

	/* Logical operations */
	Result AND(uint16_t a, uint16_t b, bool word);
	Result OR(uint16_t a, uint16_t b, bool word);
	Result XOR(uint16_t a, uint16_t b, bool word);
	Result NOT(uint16_t a, bool word);

	Result RotateLeft(uint16_t value, uint8_t count, bool word);
	Result RotateRight(uint16_t value, uint8_t count, bool word);

	Result ShiftLeft(uint16_t a, bool word);
	Result ShiftRight(uint16_t a, bool word);
}