#pragma once

#include <string>
#include <cstdint>

enum class LineType : uint16_t
{
	INFO = 1u,
	ERROR
};

struct TerminalLine
{
	std::string command{};
	LineType lineType{};

	TerminalLine(std::string_view c, LineType lt) : command{ c }, lineType{ lt } {}
};