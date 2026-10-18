#pragma once

#include "Assembler/ISAEntry.h"

#include "Debugger/Dissasembler.h"

#include "Core/CPU.h"
#include "Core/InterruptController.h"
#include "Core/Terminal.h"
#include "Core/Keyboard.h"
#include "Core/MemoryUnit.h"

#include "UI/UICommon.h"
#include "UI/InstructionTableInfo.h"

#include "Utils/StringUtils.h"
#include "Utils/Logger.h"
#include "Utils/UIUtils.h"
#include "Utils/Constants.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui-SFML.h"

#include <format>

class Assembler;
struct AssemblerError;
struct AssemblerWarning;

class Terminal;
class InterruptController;

class MemoryUnit;
class CPU;

namespace UIEditor
{
	void DrawCPUState(const CPU& cpu);
	void DrawAssemblyPanel(Mode& mode, MemoryUnit& memoryUnit,
							const Dissasembler& dissasembler, CPU& cpu, Assembler& assembler,
							Terminal& terminal, InterruptController& interruptController,
							Keyboard& keyboard, bool& executeAuto);

	void DrawMemoryView(const MemoryUnit& memoryUnit, CPU& cpu, bool& followPC);
	void DrawMenu(MemoryUnit& memoryUnit, 
				  bool& followPC, bool& executeAuto, CPU& cpu);
	void DrawSpeedSlider(float& speed);
	void DrawHelpMenu();
	void DrawOutput(const Assembler& assembler, CPU& cpu, Terminal& terminal, 
		InterruptController& interruptController, Keyboard& keyboard, bool& executeAuto);

	void DrawErrorList(std::span<const AssemblerError> errors, std::span<const AssemblerWarning> warnings);
	void DrawConsole(CPU& cpu, Terminal& terminal, InterruptController& interruptController, Keyboard& keyboard,
					bool& executeAuto);
}
