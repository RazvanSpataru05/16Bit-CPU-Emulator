#pragma once

#include <cstdint>

enum class AssemblerStage : uint8_t
{
	LEXER = 1u,
	PARSER,
	LOADER
};