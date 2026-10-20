#pragma once

#include <cstdint>

union ModRM
{
	uint8_t raw;

	struct
	{
		uint8_t rm : 3;
		uint8_t reg : 3;
		uint8_t mod : 2;
	};

	ModRM(uint8_t r) : raw{ r } {}
};