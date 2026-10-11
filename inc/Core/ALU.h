#pragma once

#include "Core/Result.h"

namespace ALU
{
	Result Add(uint8_t a, uint8_t operand, bool carry);
	Result Sub(uint8_t a, uint8_t operand, bool borrow);
	Result Inc(uint8_t a);
	Result Dec(uint8_t a);
	Result Mul(uint8_t a, uint8_t operand);

}