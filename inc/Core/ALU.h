#pragma once

#include "Core/Result.h"

namespace ALU
{
	namespace
	{
		void ComputeFlagsLogicalOperation(Result& r, uint16_t result)
		{
			r.carry = false;
			r.zero = (result == 0);
			r.negative = false;
			r.overflow = false;
		}
	}

	/* Arithmetic operations */
	Result Add(uint16_t a, uint16_t operand, bool carry);
	Result Sub(uint16_t a, uint16_t operand, bool borrow);
	Result Inc(uint16_t a);
	Result Dec(uint16_t a);
	Result Mul(uint16_t a, uint16_t operand);

	/* Logical operations */
	Result AND(uint16_t a, uint16_t operand);
	Result OR(uint16_t a, uint16_t operand);
	Result XOR(uint16_t a, uint16_t operand);
	Result NOT(uint16_t a);
	Result ShiftLeft(uint16_t a);
	Result ShiftRight(uint16_t a);

	/* Compare operation */
	Result Compare(uint16_t a, uint16_t operand);
}