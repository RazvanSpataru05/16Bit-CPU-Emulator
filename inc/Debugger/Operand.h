#pragma once

#include <cstdint>

enum class Operand : uint8_t
{
	None,
	Eb, Ev, Ew, Gb, Gv, Sw, M,	// Mod R/M
	Ib, IbSx, Iv,				// Immediate value (IbSx = signed 8-bit)
	Jb, Jv,						// Relative offset
	Ap,							// Far pointer
	Ob, Ov,						// Offset
	Reg80p, Reg160p, SegOp,		// Registers coded in opcode's bits
	AL, AX, CL, DX,				// Fixed registers
	One, Three					// Constants
};