#pragma once

#include <cstdint>

struct RMOperand
{
	bool		isReg;
	uint8_t		reg;
	uint16_t	segment;
	uint16_t	offset;
};