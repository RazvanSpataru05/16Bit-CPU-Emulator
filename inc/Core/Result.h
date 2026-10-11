#pragma once

#include <cstdint>

struct Result
{
	uint8_t value{};
	bool zero{};
	bool carry{};
	bool negative{};
	bool overflow{};

	Result() = default;
};