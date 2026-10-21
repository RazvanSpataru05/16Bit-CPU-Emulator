#pragma once

#include <cstdint>

struct Result
{
	uint16_t value{};

	bool carry{};
	bool overflow{};
	bool zero{};
	bool sign{};
	bool parity{};
	bool auxiliary{};

	Result() = default;
};