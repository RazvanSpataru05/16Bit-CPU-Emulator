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

		void SetFlag(Result& r, uint16_t mask, bool condition)
		{
			if (condition)
			{
				r.flags |= mask;
			}
			else
			{
				r.flags &= ~mask;
			}
		}

		void SetZSP(Result& r, uint32_t result, Width w)
		{
			const bool zeroCondition =		result == 0;
			const bool signCondition =		(result & w.sign) != 0;
			const bool parityCondition =	(std::popcount(static_cast<uint8_t>(result)) % 2) == 0;

			SetFlag(r, Utils::Constants::ZF_MASK, zeroCondition);
			SetFlag(r, Utils::Constants::SF_MASK, signCondition);
			SetFlag(r, Utils::Constants::PF_MASK, parityCondition);
		}

		void SetLogicFlags(Result& r, uint32_t result, Width w)
		{
			r.affected = Utils::Constants::STATUS_MASK & ~Utils::Constants::AF_MASK;
			r.flags &= ~(Utils::Constants::CF_MASK);
			r.flags &= ~(Utils::Constants::OF_MASK);
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

		const bool carryCondition = sum > w.mask;
		const bool overflowCondition = ((a ^ result) & (b ^ result) & w.sign) != 0;
		const bool auxiliaryCondition = ((a ^ b ^ result) & 0x10) != 0;

		SetFlag(r, Utils::Constants::CF_MASK, carryCondition);
		SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
		SetFlag(r, Utils::Constants::AF_MASK, auxiliaryCondition);
		SetZSP(r, result, w);

		return r;
	}

	Result Sub(uint16_t a, uint16_t b, bool borrowIn, bool word)
	{
		const Width w = W(word);
		const uint32_t bw = borrowIn ? 1u : 0u;
		const uint16_t result = static_cast<uint16_t>((static_cast<uint32_t>(a) - b - bw) & w.mask);

		Result r;
		r.value	= result;

		const bool carryCondition = static_cast<uint32_t>(a) < static_cast<uint32_t>(b) + bw;
		const bool overflowCondition = ((a ^ b) & (a ^ result) & w.sign) != 0;
		const bool auxiliaryCondition = ((a ^ b ^ result) & 0x10) != 0;

		SetFlag(r, Utils::Constants::CF_MASK, carryCondition);
		SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
		SetFlag(r, Utils::Constants::AF_MASK, auxiliaryCondition);
		SetZSP(r, result, w);

		return r;
	}

	Result INC(uint16_t a, bool word)
	{
		const Width w = W(word);
		const uint16_t result = static_cast<uint16_t>((a + 1) & w.mask);

		Result r;
		r.value	= result;
		r.affected = Utils::Constants::STATUS_MASK & ~Utils::Constants::CF_MASK;

		const bool overflowCondition = (result == w.sign);
		const bool auxiliaryCondition = ((a ^ 1 ^ result) & 0x10) != 0;

		SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
		SetFlag(r, Utils::Constants::AF_MASK, auxiliaryCondition);
		SetZSP(r, result, w);
		return r;
	}

	Result DEC(uint16_t a, bool word)
	{
		const Width w = W(word);
		const uint16_t result = static_cast<uint16_t>((a - 1) & w.mask);

		Result r;
		r.value	= result;
		r.affected = Utils::Constants::STATUS_MASK & ~Utils::Constants::CF_MASK;

		const bool overflowCondition = (a == w.sign);
		const bool auxiliaryCondition = ((a ^ 1 ^ result) & 0x10) != 0;

		SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
		SetFlag(r, Utils::Constants::AF_MASK, auxiliaryCondition);
		SetZSP(r, result, w);
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

		const bool carryCondition = (a & w.sign) != 0;
		SetFlag(r, Utils::Constants::CF_MASK, carryCondition);

		return r;
	}

	Result ShiftRight(uint16_t a, bool word)
	{
		const Width w = W(word);
		const uint16_t result = (a >> 1) & w.mask;
		Result r;

		r.value = result;
		SetLogicFlags(r, result, w);

		const bool carryCondition = (a & 0x01) != 0;
		SetFlag(r, Utils::Constants::CF_MASK, carryCondition);

		return r;
	}
}