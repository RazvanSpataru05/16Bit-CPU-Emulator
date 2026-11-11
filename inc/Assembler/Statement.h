#pragma once

#include "Assembler/ISAEntry.h"

#include "Core/uint20_t.h"

#include <array>
#include <cstdint>
#include <optional>

struct Statement
{
	std::optional<uint8_t> opcode{ std::nullopt };
	std::optional<uint8_t> modRM{ std::nullopt };

	uint20_t address{};
	uint8_t length{}; // 1-6 based on the instruction
	uint8_t operandCount{};

	const ISA::ISAEntry* ISAEntry = nullptr; 

	Statement() = default;
};