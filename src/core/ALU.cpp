#include "Core/ALU.h"

namespace ALU
{

	Result Add(uint16_t a, uint16_t operand, bool carry)
	{
		uint16_t sum = a + operand + (carry ? 1 : 0);
		uint16_t result = static_cast<uint16_t>(sum);

		Result r;
		r.value = result;
		r.carry = sum > 0xFF;
		r.zero = (result == 0);
		r.negative = (result & 0x80) != 0;
		r.overflow = (~(a ^ operand) & (a ^ result) & 0x80) != 0;

		return r;
	}
	Result Sub(uint16_t a, uint16_t operand, bool borrow)
	{
		uint16_t diff = a - operand - (borrow ? 1 : 0);
		uint16_t result = static_cast<uint16_t>(diff);

		Result r;
		r.value = result;
		r.carry = diff > 0xFF;
		r.zero = (result == 0);
		r.negative = (result & 0x80) != 0;
		r.overflow = ((a ^ operand) & (a ^ result) & 0x80) != 0;

		return r;
	}

	Result Inc(uint16_t a)
	{
		uint16_t result = static_cast<uint16_t>(a + 1);

		Result r;
		r.value = result;
		r.zero = (result == 0);
		r.negative = (result & 0x80) != 0;
		r.overflow = (~(a ^ 1) & (a ^ result) & 0x80) != 0;

		return r;
	}

	Result Dec(uint16_t a)
	{
		uint16_t result = static_cast<uint16_t>(a - 1);

		Result r;
		r.value = result;
		r.zero = (result == 0);
		r.negative = (result & 0x80) != 0;
		r.overflow = ((a ^ 1) & (a ^ result) & 0x80) != 0;

		return r;
	}

	Result Mul(uint16_t a, uint16_t operand)
	{
		uint16_t prod = a * operand;
		uint16_t result = static_cast<uint16_t>(prod);

		Result r;
		r.value = result;
		r.carry = prod > 0xFF;
		r.zero = (result == 0);
		r.negative = (result & 0x80) != 0;
		r.overflow = false; // no explicit definiton for overflow flag without hardware native support

		return r;
	}

	Result AND(uint16_t a, uint16_t operand)
	{
		uint16_t result = a & operand;
		Result r;

		r.value = result;
		ComputeFlagsLogicalOperation(r, result);

		return r;
	}

	Result OR(uint16_t a, uint16_t operand)
	{
		uint16_t result = a | operand;
		Result r;

		r.value = result;
		ComputeFlagsLogicalOperation(r, result);

		return r;
	}

	Result XOR(uint16_t a, uint16_t operand)
	{
		uint16_t result = a ^ operand;
		Result r;

		r.value = result;
		ComputeFlagsLogicalOperation(r, result);

		return r;
	}

	Result NOT(uint16_t a)
	{
		uint16_t result = ~a;
		Result r;

		r.value = result;
		ComputeFlagsLogicalOperation(r, result);

		return r;
	}

	Result ShiftLeft(uint16_t a)
	{
		uint16_t result = a << 1;
		Result r;

		r.value = result;
		ComputeFlagsLogicalOperation(r, result);
		r.carry = (a & 0x80) != 0;

		return r;
	}

	Result ShiftRight(uint16_t a)
	{
		uint16_t result = a >> 1;
		Result r;

		r.value = result;
		ComputeFlagsLogicalOperation(r, result);
		r.carry = (a & 0x01) != 0;

		return r;
	}

	Result Compare(uint16_t a, uint16_t operand)
	{
		uint16_t result = a - operand;
		Result r;

		r.value = result;
		r.carry = operand > a;
		r.zero = (result == 0);
		r.negative = (result & 0x80) != 0;
		r.overflow = ((a ^ operand) & (a ^ result) & 0x80) != 0;

		return r;
	}
}