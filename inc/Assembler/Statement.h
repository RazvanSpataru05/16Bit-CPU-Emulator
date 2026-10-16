#pragma once

#include "Assembler/ISAEntry.h"

#include <array>
#include <cstdint>
#include <optional>

struct Statement
{
	std::optional<uint16_t> opcode = std::nullopt;
	uint16_t address{};
	uint16_t operatorCount{};
	std::array<uint16_t, 2> operands{};
	const ISA::ISAEntry* ISAEntry = nullptr; 

	Statement() = default;
};