#pragma once

#include "Assembler/ISAEntry.h"

#include "UI/UICommon.h"
#include "UI/InstructionTableInfo.h"

#include "Utils/StringUtils.h"

#include "imgui.h"

#include <format>

using namespace UIEditor;

namespace Utils
{
	namespace UI
	{
		bool ErrorListTab(SelectedTab tab);
		bool ConsoleListTab(SelectedTab tab);

		void DisplayInstruction(const InstructionDef& instruction, MemoryUnit& memoryUnit, uint16_t index, std::optional<uint8_t> opcode);

		void DisplayGlossaryPage();
		void DisplayMiscPage(const char* title, const char* tableId, std::initializer_list<uint8_t> opcodes);
		void DisplayTable(const InstructionTableInfo& info);
	}	
}