#pragma once

#include "Core/Result.h"

namespace ALU
{
	namespace
	{
		void ComputeFlagsLogicalOperation(Result& r, uint8_t result)
		{
			r.carry = false;
			r.zero = (result == 0);
			r.negative = false;
			r.overflow = false;
		}
	}

	/* Arithmetic operations */
	Result Add(uint8_t a, uint8_t operand, bool carry);
	Result Sub(uint8_t a, uint8_t operand, bool borrow);
	Result Inc(uint8_t a);
	Result Dec(uint8_t a);
	Result Mul(uint8_t a, uint8_t operand);

	/* Logical operations */
	Result And(uint8_t a, uint8_t operand);
	Result Or(uint8_t a, uint8_t operand);
	Result Xor(uint8_t a, uint8_t operand);
	Result Not(uint8_t a);
	Result ShiftLeft(uint8_t a);
	Result ShiftRight(uint8_t a);

	/* Compare operation */
	Result Compare(uint8_t a, uint8_t operand);
}