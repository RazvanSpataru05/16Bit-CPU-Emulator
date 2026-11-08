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
		case 0b000: return ADD(a, b, false,		word); // ADD
		case 0b001: return OR(a, b,				word); // OR
		case 0b010: return ADD(a, b, carryIn,	word); // ADC 
		case 0b011: return SUB(a, b, carryIn,	word); // SBB
		case 0b100: return AND(a, b,			word); // AND
		case 0b101: return SUB(a, b, false,		word); // SUB
		case 0b110: return XOR(a, b,			word); // XOR
		case 0b111: return SUB(a, b, false,		word); // CMP
		}
	}

	Result Shift(uint8_t operation, uint16_t value, uint8_t count, bool word, bool carryIn)
	{
		switch (operation)
		{
		case 0b000:		return RotateLeft(value, count, word);						// ROL
		case 0b001:		return RotateRight(value, count, word);						// ROR
		case 0b010:		return RotateCarryLeft(value, count, word, carryIn);		// RCL
		case 0b011:		return RotateCarryRight(value, count, word, carryIn);		// RCR
		case 0b100:		return ShiftLeft(value, count, word, carryIn);				// SHL
		case 0b101:		return ShiftRight(value, count, word, carryIn);				// SHR
		case 0b110:		return SetMinusOne(word);									// SETMO (Undefined)
		case 0b111:		return ShiftRightArithmetic(value, count, word, carryIn);	// SAR
		}
	}

	Result ADD(uint16_t a, uint16_t b, bool carryIn, bool word)
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

	Result SUB(uint16_t a, uint16_t b, bool borrowIn, bool word)
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

	Result RotateLeft(uint16_t value, uint8_t count, bool word)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = static_cast<uint16_t>(value & w.mask);
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % bits;
		const uint32_t input = value & w.mask;

		const uint32_t result = effective == 0u
			? input
			: ((input << effective) | (input >> (bits - effective))) & w.mask;

		const bool carryCondition = (result & 1u) != 0;
		SetFlag(r, Utils::Constants::CF_MASK, carryCondition);

		if (count == 1u)
		{
			const bool overflowCondition = ((result & w.sign) != 0) != carryCondition;
			SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
		}

		r.value = static_cast<uint16_t>(result);
		r.affected = Utils::Constants::CF_MASK | Utils::Constants::OF_MASK;
		return r;
	}

	Result RotateRight(uint16_t value, uint8_t count, bool word)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = static_cast<uint16_t>(value & w.mask);
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % bits;
		const uint32_t input = value & w.mask;

		const uint32_t result = effective == 0u
			? input
			: ((input >> effective) | (input << (bits - effective))) & w.mask;

		const bool carryCondition = (result & w.sign) != 0;
		SetFlag(r, Utils::Constants::CF_MASK, carryCondition);

		if (count == 1u)
		{
			const bool topBit	 =		(result & w.sign)		!= 0;
			const bool secondBit =		(result & (w.sign >> 1)) != 0;
			SetFlag(r, Utils::Constants::OF_MASK, topBit != secondBit);
		}

		r.value = static_cast<uint16_t>(result);
		r.affected = Utils::Constants::CF_MASK | Utils::Constants::OF_MASK;
		return r;
	}

	Result RotateCarryLeft(uint16_t value, uint8_t count, bool word, bool carryIn)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = static_cast<uint16_t>(value & w.mask);
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % (bits + 1u);
		uint32_t result = value & w.mask;
		bool carry = carryIn;

		for (uint8_t i = 0u; i < effective; ++i)
		{
			const bool topBit = (result & w.sign) != 0;
			result = ((result << 1) | (carry ? 1u : 0u)) & w.mask;
			carry = topBit;
		}

		SetFlag(r, Utils::Constants::CF_MASK, carry);
		r.affected = Utils::Constants::CF_MASK;
		
		if (count == 1u)
		{
			const bool overflowCondition = ((result & w.sign) != 0) != carry;
			SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
			r.affected |= Utils::Constants::OF_MASK;
		}

		r.value = static_cast<uint16_t>(result);
		return r;
	}

	Result RotateCarryRight(uint16_t value, uint8_t count, bool word, bool carryIn)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = static_cast<uint16_t>(value & w.mask);
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % (bits + 1u);
		uint32_t result = value & w.mask;
		bool carry = carryIn;

		for (uint8_t i = 0u; i < effective; ++i)
		{
			const bool lowBit = (result & 1u) != 0;
			result >>= 1;
			if (carry) result |= w.sign;
			carry = lowBit;
		}

		SetFlag(r, Utils::Constants::CF_MASK, carry);
		r.affected = Utils::Constants::CF_MASK;

		if (count == 1u)
		{
			const bool topBit    =		(result & w.sign)		 != 0;
			const bool secondBit =		(result & (w.sign >> 1)) != 0;
			SetFlag(r, Utils::Constants::OF_MASK, topBit != secondBit);
			r.affected |= Utils::Constants::OF_MASK;
		}

		r.value = static_cast<uint16_t>(result);
		return r;
	}

	Result ShiftLeft(uint16_t value, uint8_t count, bool word, bool carryIn)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = value & w.mask;
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % bits;
		uint32_t result = value & w.mask;
		bool carry = carryIn;

		for (uint8_t i = 0u; i < effective; ++i)
		{
			const bool topBit = (result & w.sign) != 0;
			result <<= 1;
			carry = topBit;
		}

		SetFlag(r, Utils::Constants::CF_MASK, carry);
		r.affected |= Utils::Constants::CF_MASK;

		if (count == 1u)
		{
			const bool overflowCondition = ((result & w.sign) != 0) != carry;
			SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
			r.affected |= Utils::Constants::OF_MASK;
		}

		r.value = static_cast<uint16_t>(result);
		return r;
	}

	Result ShiftRight(uint16_t value, uint8_t count, bool word, bool carryIn)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = value & w.mask;
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % bits;
		const uint32_t input = value & w.mask;
		uint32_t result = input;
		bool carry = carryIn;

		for (uint8_t i = 0u; i < effective; ++i)
		{
			const bool lowBit = (result & 1u) != 0;
			result >>= 1;
			carry = lowBit;
		}

		SetFlag(r, Utils::Constants::CF_MASK, carry);
		r.affected |= Utils::Constants::CF_MASK;

		if (count == 1u)
		{
			const bool overflowCondition = (input & w.sign) != 0;
			SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
			r.affected |= Utils::Constants::OF_MASK;
		}

		r.value = static_cast<uint16_t>(result);
		return r;
	}

	Result SetMinusOne(bool word)
	{
		const Width w = W(word);
		Result r;
		r.value = w.mask;

		SetFlag(r, Utils::Constants::ZF_MASK, false);
		SetFlag(r, Utils::Constants::SF_MASK, true);
		SetFlag(r, Utils::Constants::PF_MASK, true);
		r.affected = Utils::Constants::ZF_MASK
				   | Utils::Constants::SF_MASK
				   | Utils::Constants::PF_MASK;

		return r;
	}

	Result ShiftRightArithmetic(uint16_t value, uint8_t count, bool word, bool carryIn)
	{
		const Width w = W(word);
		const uint32_t bits = word ? 16u : 8u;
		Result r;

		if (count == 0u)
		{
			r.value = value & w.mask;
			r.affected = 0;
			return r;
		}

		const uint32_t effective = count % bits;
		const uint32_t input = value & w.mask;
		uint32_t result = input;
		bool carry = carryIn;

		for (uint8_t i = 0u; i < effective; ++i)
		{
			const bool topBit = (result & w.sign) != 0;
			const bool lowBit = (result & 1u) != 0;
			result >>= 1;
			if (topBit) result |= w.sign;

			carry = lowBit;
		}

		SetFlag(r, Utils::Constants::CF_MASK, carry);
		r.affected |= Utils::Constants::CF_MASK;

		if (count == 1u)
		{
			const bool overflowCondition = (input & w.sign) != 0;
			SetFlag(r, Utils::Constants::OF_MASK, overflowCondition);
			r.affected |= Utils::Constants::OF_MASK;
		}

		r.value = static_cast<uint16_t>(result);
		return r;
	}
}