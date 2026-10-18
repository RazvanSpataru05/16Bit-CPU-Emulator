#pragma once

#include "Core/uint20_t.h"

struct AddressValue
{
	uint20_t address{};
	uint8_t value{};

	AddressValue(uint20_t a, uint8_t v) :
		address{ a }, value{ v }
	{
	}
};