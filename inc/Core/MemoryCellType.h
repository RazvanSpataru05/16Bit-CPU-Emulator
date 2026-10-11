#pragma once

#include <cstdint>

enum class MemoryCellType : uint8_t
{
	UNKNOWN = 0u,
	OPCODE,
	OPERAND,
	DATA
};