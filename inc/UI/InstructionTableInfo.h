#pragma once

#include <cstdint>

struct InstructionTableInfo
{
	const char* title = nullptr;
	const char* tableId = nullptr;
	uint16_t firstInstruction{};
	uint16_t lastInstruction{};


	constexpr InstructionTableInfo(const char* t, const char* id, uint16_t first, uint16_t last) :
		title{ t }, tableId{ id }, firstInstruction{ first }, lastInstruction{ last }
	{

	}
};