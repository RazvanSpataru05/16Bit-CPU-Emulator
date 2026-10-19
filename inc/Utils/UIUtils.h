#pragma once

#include "Assembler/ISAEntry.h"

#include "Core/MemoryUnit.h"

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

		void DisplayInstruction(const InstructionDef& instruction, MemoryUnit& memoryUnit, uint16_t index, std::optional<uint16_t> opcode);

		void DisplayGlossaryPage();
		void DisplayMiscPage(const char* title, const char* tableId, std::initializer_list<uint16_t> opcodes);
		void DisplayTable(const InstructionTableInfo& info);

		ImVec2 CalcMenuWindowPos();
		ImVec2 CalcMenuWindowSize();

		ImVec2 CalcMemoryViewPos();
		ImVec2 CalcMemoryViewSize();

		ImVec2 CalcCodePanelPos();
		ImVec2 CalcCodePanelSize();

		ImVec2 CalcTextEditorSize();
		ImVec2 CalcTabSize();
	}	
}