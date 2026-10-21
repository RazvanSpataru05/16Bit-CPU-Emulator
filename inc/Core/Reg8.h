#pragma once

#include <cstdint>

enum class Reg8 : uint8_t
{
	AL = 0x00,				// AX low byte
	CL,						// CX low byte
	DL,						// DX low byte
	BL,						// BL high byte
	AH,						// AX high byte
	CH,						// CX high byte
	DH,						// DX high byte
	BH						// BX high byte
};