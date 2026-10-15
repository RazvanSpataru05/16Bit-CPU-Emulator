#pragma once

#include "Assembler/TokenType.h"

struct Token
{
	TokenType type{};
	std::string value{};
	uint32_t line{};
	uint32_t column{};

	Token(TokenType t, std::string_view v, uint32_t l, uint32_t c) :
		type{ t }, value{ v }, line{ l }, column{ c } {
	}
};