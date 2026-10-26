#pragma once

#include "Utils/Constants.h"

#include <cstdint>

struct Result
{
	uint16_t value{};
	uint16_t flags{};
	uint16_t affected = Utils::Constants::STATUS_MASK;

	Result() = default;
};