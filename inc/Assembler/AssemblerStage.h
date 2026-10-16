#pragma once

#include <cstdint>

enum class AssemblerStage : uint16_t
{
	LEXER = 1u,
	PARSER,
	LOADER
};