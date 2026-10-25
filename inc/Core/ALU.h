#pragma once

#include "Core/Result.h"

#include <bit>

namespace ALU
{
	Result Execute(uint8_t operation, uint16_t a, uint16_t b, bool word, bool carryIn);

	Result Add(uint16_t a, uint16_t b, bool carryIn, bool word);
	Result Sub(uint16_t a, uint16_t b, bool borrowIn, bool word);

	Result INC(uint16_t a, bool word);
	Result DEC(uint16_t a, bool word);
	Result Mul(uint16_t a, uint16_t b);

	/* Logical operations */
	Result AND(uint16_t a, uint16_t b, bool word);
	Result OR(uint16_t a, uint16_t b, bool word);
	Result XOR(uint16_t a, uint16_t b, bool word);
	Result NOT(uint16_t a, bool word);

	Result ShiftLeft(uint16_t a, bool word);
	Result ShiftRight(uint16_t a, bool word);

	/* Compare operation */
	Result Cmp(uint16_t a, uint16_t b);
}