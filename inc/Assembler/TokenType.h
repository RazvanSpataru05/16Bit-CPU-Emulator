#pragma once

#include <cstdint>

enum class TokenType : uint16_t
{
	NONE = 0u,

	MNEMONIC,
	REGISTER, // A-D or a-d
	NUMBER,
	NEW_LINE,
	IDENTIFIER,

	// operators
	PLUS,
	MINUS,
	STAR,
	SLASH,

	LEFT_PARAN,
	RIGHT_PARAN,
	LEFT_BRACKET,
	RIGHT_BRACKET,

	COLON,
	COMMA,
	DOT,

	DW, // define word

	END_OF_FILE
};