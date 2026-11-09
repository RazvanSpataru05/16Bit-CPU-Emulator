#pragma once

#include <cstdint>

struct RepeatState
{
	uint16_t IP{};
	bool repeatIfZero{};
	bool canGoBack{};

	void Reset()
	{
		IP = 0x0000;
		repeatIfZero = false;
		canGoBack = false;
	}

	RepeatState() = default;
};
