#pragma once

#include <cstdint>

enum class Reg16 : uint8_t
{
	AX = 0x00,		// Accumulator Reg
	CX,				// Counter Reg
	DX,				// Data Reg
	BX,				// Base Reg

	SP,				// Stack Pointer
	BP,				// Base Pointer
	SI,				// Source Index Reg
	DI				// Destination Index Reg
};