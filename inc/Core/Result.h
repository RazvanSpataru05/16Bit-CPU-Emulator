#pragma once

#include <cstdint>

struct Result
{
	uint16_t value{};
	bool zero{};
	bool carry{};
	bool negative{};
	bool overflow{};

	Result() = default;
};