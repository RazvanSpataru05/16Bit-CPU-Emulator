#pragma once

#include <cstdint>

enum class Slot : uint8_t
{
	NONE = 0u,
	IN_OPCODE,
	MOD_RM_REG,
	MOD_RM_RM,
	IMM
};