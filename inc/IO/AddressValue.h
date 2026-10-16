#pragma once

#include <cstdint>

struct AddressValue
{
	uint16_t address{};
	uint16_t value{};

	AddressValue(uint16_t a, uint16_t v) :
		address{ a }, value{ v }
	{
	}
};