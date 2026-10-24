#include "Core/ALU.h"

namespace ALU
{
	namespace
	{
		struct Width
		{
			uint32_t mask;
			uint32_t sign;
		};

		constexpr Width W(bool word)
		{
			return word ? Width{ 0xFFFF, 0x8000 } : Width{ 0xFF, 0x80 };
		}

		void SetZSP(Result& r, uint32_t result, Width w)
		{
			r.zero		= result == 0;
			r.sign		= (result & w.sign) != 0;
			r.parity	= (std::popcount(static_cast<uint8_t>(result)) % 2) == 0;
		}

		void SetLogicFlags(Result& r, uint32_t result, Width w)
		{
			r.carry = false;
			r.overflow = false;
			SetZSP(r, result, w);
		}
	}

	Result Execute(uint8_t operation, uint16_t a, uint16_t b, bool word, bool carryIn)
	{
		switch (operation) 
		{
		case 0: return Add(a, b, false,		word); // ADD
		case 1: return OR(a, b,				word); // OR
		case 2: return Add(a, b, carryIn,	word); // ADC 
		case 3: return Sub(a, b, carryIn,	word); // SBB
		case 4: return AND(a, b,			word); // AND
		case 5: return Sub(a, b, false,		word); // SUB
		case 6: return XOR(a, b,			word); // XOR
		case 7: return Sub(a, b, false,		word); // CMP
		}
	}

	Result Add(uint16_t a, uint16_t b, bool carryIn, bool word)
	{
		const Width w = W(word);
		const uint32_t sum = static_cast<uint32_t>(a) + b + (carryIn ? 1u : 0u);
		const uint16_t result = static_cast<uint16_t>(sum & w.mask);
		
		Result r;
		r.value		= result;
		r.carry		= sum > w.mask;
		r.overflow	= ((a ^ result) & (b ^ result) & w.sign) != 0;
		r.auxiliary = ((a ^ b ^ result) & 0x10) != 0;
		SetZSP(r, result, w);

		return r;
	}

	Result Sub(uint16_t a, uint16_t b, bool borrowIn, bool word)
	{
		const Width w = W(word);
		const uint32_t bw = borrowIn ? 1u : 0u;
		const uint16_t result = static_cast<uint16_t>(static_cast<uint32_t>(a) - b - bw & w.mask);

		Result r;
		r.value		= result;
		r.carry		= static_cast<uint32_t>(a) < static_cast<uint32_t>(b) + bw;
		r.overflow	= ((a ^ b) & (a ^ result) & w.sign) != 0;
		r.auxiliary = ((a ^ b ^ result) & 0x10) != 0;
		SetZSP(r, result, w);

		return r;
	}

	Result Inc(uint16_t a)
	{
		uint16_t result = static_cast<uint16_t>(a + 1);

		Result r;
		r.value = result;
		r.zero = (result == 0);
		r.sign = (result & 0x80) != 0;
		r.overflow = (~(a ^ 1) & (a ^ result) & 0x80) != 0;

		return r;
	}

	Result Dec(uint16_t a)
	{
		uint16_t result = static_cast<uint16_t>(a - 1);

		Result r;
		r.value = result;
		r.zero = (result == 0);
		r.sign = (result & 0x80) != 0;
		r.overflow = ((a ^ 1) & (a ^ result) & 0x80) != 0;

		return r;
	}

	Result Mul(uint16_t a, uint16_t b)
	{
		uint16_t prod = a * b;
		uint16_t result = static_cast<uint16_t>(prod);

		Result r;
		r.value = result;
		r.carry = prod > 0xFF;
		r.zero = (result == 0);
		r.sign = (result & 0x80) != 0;
		r.overflow = false; // no explicit definiton for overflow flag without hardware native support

		return r;
	}

	Result AND(uint16_t a, uint16_t b, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (a & b) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);

		return r;
	}

	Result OR(uint16_t a, uint16_t b, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (a | b) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);

		return r;
	}

	Result XOR(uint16_t a, uint16_t b, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (a ^ b) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);

		return r;
	}

	Result NOT(uint16_t a, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (~a) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);

		return r;
	}

	Result ShiftLeft(uint16_t a, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (a << 1) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);
		r.carry = (a & w.mask) != 0;

		return r;
	}

	Result ShiftRight(uint16_t a, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (a >> 1) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);
		r.carry = (a & 0x01) != 0;

		return r;
	}

	Result Cmp(uint16_t a, uint16_t b)
	{
		uint16_t result = a - b;
		Result r;

		r.value = result;
		r.carry = b > a;
		r.zero = (result == 0);
		r.sign = (result & 0x80) != 0;
		r.overflow = ((a ^ b) & (a ^ result) & 0x80) != 0;

		return r;
	}
}