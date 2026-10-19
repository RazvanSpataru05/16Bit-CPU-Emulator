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
#include "UI/AppConfig.h"

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

class Emulator;

class Terminal;
class InterruptController;

class MemoryUnit;
class CPU;

namespace UIEditor
{
	void DrawCPUState(const CPU& cpu);
	void DrawCodePanel(Emulator& emulator, AppConfig& config);
	void DrawCPUDiagram();
	void DrawDissasemblyPanel(MemoryUnit& memoryUnit, const CPU& cpu, const Dissasembler& dissasembler);

	void DrawMemoryView(Emulator& emulator, AppConfig& config);
	void DrawMenu(Emulator& emulator, AppConfig& config);
	void DrawHelpMenu();
	void DrawOutput(Emulator& emulator, AppConfig& config);

	void DrawErrorList(std::span<const AssemblerError> errors, std::span<const AssemblerWarning> warnings);
	void DrawConsole(Emulator& emulator, AppConfig& config);
}
