#pragma once

#include <cstdint>

enum class MemoryCellType : uint16_t
{
	UNKNOWN = 0u,
	OPCODE,
	OPERAND,
	DATA
};