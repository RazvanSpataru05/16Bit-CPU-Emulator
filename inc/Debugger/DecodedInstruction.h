#pragma once

#include <string>
#include <cstdint>

struct DecodedInstruction
{
	std::string text;
	uint16_t length{ 1u };
};