#include "UI/UIEditor.h"

#include "Assembler/Assembler.h"

namespace UIEditor
{
	namespace
	{
		SelectedTab tab{ SelectedTab::ERROR_LIST };
		PageType currentHelperPage{ PageType::GLOSSARY_PAGE };

		constexpr uint16_t PAGE_SIZE = 256u;

		constexpr uint8_t FIRST_LOAD_INSTRUCTION = 0x01;
		constexpr uint8_t LAST_LOAD_INSTRUCTION = 0x14;

		constexpr uint8_t FIRST_ARITHMETIC_INSTRUCTION = 0x20;
		constexpr uint8_t LAST_ARITHMETIC_INSTRUCTION = 0x2B;

		constexpr uint8_t FIRST_LOGIC_INSTRUCTION = 0x30;
		constexpr uint8_t LAST_LOGIC_INSTRUCTION = 0x38;

		constexpr uint8_t FIRST_COMPARE_INSTRUCTION = 0x40;
		constexpr uint8_t LAST_COMPARE_INSTRUCTION = 0x41;

		constexpr uint8_t FIRST_JUMP_INSTRUCTION = 0x50;
		constexpr uint8_t LAST_JUMP_INSTRUCTION = 0x58;

		constexpr uint8_t FIRST_STACK_INSTRUCTION = 0x60;
		constexpr uint8_t LAST_STACK_INSTRUCTION = 0x63;

		constexpr InstructionTableInfo INSTRUCTION_TABLES[] =
		{
				{ "Load & Store Instructions Table", "##load_store_table",	FIRST_LOAD_INSTRUCTION,			LAST_LOAD_INSTRUCTION },
				{ "Arithmetic Instructions Table",   "##arithmetic_table",	FIRST_ARITHMETIC_INSTRUCTION,	LAST_ARITHMETIC_INSTRUCTION },
				{ "Logical Instructions Table",      "##logical_table",		FIRST_LOGIC_INSTRUCTION,		LAST_LOGIC_INSTRUCTION },
				{ "Compare Instructions Table",      "##compare_table",		FIRST_COMPARE_INSTRUCTION,		LAST_COMPARE_INSTRUCTION },
				{ "Jump Instructions Table",         "##jump_table",		FIRST_JUMP_INSTRUCTION,			LAST_JUMP_INSTRUCTION },
				{ "Stack Instructions Table",        "##stack_table",		FIRST_STACK_INSTRUCTION,		LAST_STACK_INSTRUCTION }
		};

		const size_t BUFFER_SIZE{ 8192 };
		const size_t CONSOLE_BUFFER_SIZE{ 256 };

		uint8_t currentPage{};

		bool helpMenuVisibility{ false };

		void NextPage()
		{
			if (currentHelperPage != PageType::MISC_PAGE)
			{
				currentHelperPage = static_cast<PageType>(static_cast<uint8_t>(currentHelperPage) + 1);
			}
		}

		void PreviousPage()
		{
			if (currentHelperPage != PageType::GLOSSARY_PAGE)
			{
				currentHelperPage = static_cast<PageType>(static_cast<uint8_t>(currentHelperPage) - 1);
			}
		}
	}

	void DrawCPUState(const CPU& cpu)
	{
		ImGui::Begin("CPU State");
		ImGui::Text(std::format("Program Counter: {:#06X}",			cpu.GetPC()).c_str());
		ImGui::Text(std::format("Stack Pointer: {:#06X}",			cpu.GetSP()).c_str());
		ImGui::Text(std::format("Instruction Register: {:#04X}",	cpu.GetIR()).c_str());
		ImGui::Separator();

		ImGui::Text("\nRegistry");
		ImGui::Text(std::format("A: {}", cpu.GetRegisterFile().A()).c_str());
		ImGui::Text(std::format("B: {}", cpu.GetRegisterFile().B()).c_str());
		ImGui::Text(std::format("C: {}", cpu.GetRegisterFile().C()).c_str());
		ImGui::Text(std::format("D: {}", cpu.GetRegisterFile().D()).c_str());
		ImGui::Separator();

		ImGui::Text("\nFlags");
		ImGui::Text(std::format("Interrupt Flag: {}",	cpu.GetInterruptController().GetInterruptFlag() ? "True" : "False").c_str());
		ImGui::Text(std::format("Zero Flag: {}",		cpu.GetStatusRegister().Zero() ? "True" : "False").c_str());
		ImGui::Text(std::format("Carry Flag: {}",		cpu.GetStatusRegister().Carry() ? "True" : "False").c_str());
		ImGui::Text(std::format("Negative Flag: {}",	cpu.GetStatusRegister().Negative() ? "True" : "False").c_str());
		ImGui::Text(std::format("Overflow Flag: {}",	cpu.GetStatusRegister().Overflow() ? "True" : "False").c_str());
		ImGui::Text(std::format("Halt Flag: {}",		cpu.GetHaltFlag() ? "True" : "False").c_str());
		ImGui::Separator();
		ImGui::End();
	}

	void DrawAssemblyPanel(Mode& mode, MemoryUnit& memoryUnit, const Dissasembler& dissasembler,
		CPU& cpu, Assembler& assembler, Terminal& terminal, InterruptController& interruptController,
		Keyboard& keyboard, bool& executeAuto)
	{
		static char editorBuffer[BUFFER_SIZE];

		ImGui::Begin("Assembly Panel");
		if (ImGui::Button("Edit Mode"))
		{
			mode = Mode::EDIT;
		}
		ImGui::SameLine();
		if (ImGui::Button("Dissasembly"))
		{
			mode = Mode::DISSASEMBLY;
		}
		ImGui::Separator();

		if (mode == Mode::EDIT)
		{
			ImGui::InputTextMultiline
			("##editor", editorBuffer, IM_ARRAYSIZE(editorBuffer), ImVec2(-1, 300), ImGuiInputTextFlags_AllowTabInput);

			if (ImGui::Button("Assemble & Load"))
			{
				Logger::ClearLogFile();
				if (assembler.Assemble(editorBuffer))
				{
					const auto& statements = assembler.GetStatements();
					const Program program = DataLoader::ParseStatements(statements, assembler.GetStartAddress());

					memoryUnit.LoadValuesIntoMemory(program);
					std::cout << "Start address: " << memoryUnit.GetStartAddress() << std::endl;
					cpu.SetProgramCounter(memoryUnit.GetStartAddress());

					tab = SelectedTab::CONSOLE;
					mode = Mode::DISSASEMBLY;
				}
				else
				{
					tab = SelectedTab::ERROR_LIST;
				}
			}
			DrawOutput(assembler, cpu, terminal, interruptController, keyboard, executeAuto);
		}

		else if (mode == Mode::DISSASEMBLY)
		{
			if (!memoryUnit.IsMemoryEmpty())
			{
				for (size_t index = 0; index < MEMORY_SIZE; ++index)
				{
					std::optional<uint8_t> opcode = memoryUnit.Peek(static_cast<uint16_t>(index));
					if (!opcode.has_value())
					{
						continue;
					}

					const InstructionDef& instruction = dissasembler.GetInstructionDef(opcode.value());

					if (index == cpu.GetPC())
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
					}
					else
					{
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
					}
						
					Utils::UI::DisplayInstruction(instruction, memoryUnit, static_cast<uint16_t>(index), opcode);

					index += static_cast<size_t>(instruction.size - 1);
					ImGui::PopStyleColor();
				}
			}
		}
		ImGui::End();
	}

	void DrawMemoryView(const MemoryUnit& memoryUnit, CPU& cpu, bool& followPC)
	{
		if (followPC)
		{
			currentPage = static_cast<uint8_t>(cpu.GetPC() / PAGE_SIZE);
		}

		ImGui::Begin(std::format("Memory View (Page {}/{})", currentPage, PAGE_SIZE-1).c_str());
		if (ImGui::Button("\t\t\tPrev\t\t\t"))
		{
			currentPage = currentPage - 1 < 0 ? 255 : currentPage - 1;
		}

		ImGui::SameLine();
		const std::string spaces = std::string(" ", 79);
		ImGui::Text(spaces.c_str());
		ImGui::SameLine();

		if (ImGui::Button("\t\t\tNext\t\t\t"))
		{
			currentPage = (currentPage + 1) % 256;
		}

		const size_t startAddress = static_cast<size_t>(currentPage * PAGE_SIZE);
		const size_t endAddress = startAddress + PAGE_SIZE;

		if (ImGui::BeginTable("MemoryTable", 16, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			for (size_t index = startAddress; index < endAddress; ++index)
			{
				ImGui::TableNextColumn();
				if (index == cpu.GetPC())
				{
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 0, 255, 255));
				}

				if (cpu.IsReadingInstruction() && index == cpu.ComputeAddress(cpu.GetPC() + 1))
				{
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 255, 0, 255));
				}
				else if (cpu.IsWritingInstruction() &&
					index == cpu.ComputeAddress(cpu.GetPC() + 1))
				{
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 0, 0, 255));
				}

				char cellText[8];
				if (const auto result = memoryUnit.Peek(static_cast<uint16_t>(index)); result.has_value())
				{
					snprintf(cellText, sizeof(cellText), "0x%02X", result.value());
				}
				else
				{
					snprintf(cellText, sizeof(cellText), "--");
				}

				const float cellWidth = ImGui::GetContentRegionAvail().x;
				const float textWidth = ImGui::CalcTextSize(cellText).x;

				if (cellWidth > textWidth)
				{
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (cellWidth - textWidth) * 0.5f);
				}
				ImGui::Text(cellText);
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
	void DrawMenu(MemoryUnit& memoryUnit, bool& followPC, bool& executeAuto,  CPU& cpu)
	{

		ImGui::Begin("Menu");
		if (ImGui::Button("\t\tNext Step\t\t"))
		{
			cpu.Step();
		}
		ImGui::SameLine();

		if (ImGui::Button("\t\tReset\t\t"))
		{
			memoryUnit.Clear();
			cpu.Reset();
			executeAuto = false;
			followPC = true;
		}

		if (ImGui::Button(std::format("Auto ({})", executeAuto ? "ON" : "OFF").c_str()))
		{
			executeAuto = !executeAuto;
		}
		ImGui::SameLine();

		if (ImGui::Button(std::format("Follow PC ({})", followPC ? "ON" : "OFF").c_str()))
		{
			followPC = !followPC;
		}
		ImGui::End();

		ImGui::Begin("Help");
		if (ImGui::Button("Help"))
		{
			helpMenuVisibility = true;
		}
		ImGui::End();
	}

	void DrawSpeedSlider(float& speed)
	{
		ImGui::Begin("Speed Slider");
		ImGui::SliderFloat("Speed (seconds)", &speed, 0.1f, 2.0f);
		ImGui::End();
	}

	void DrawHelpMenu()
	{
		if (helpMenuVisibility)
		{
			ImGui::Begin("Help Menu");

			const std::string pageText = std::format("Page {}/8", static_cast<uint8_t>(currentHelperPage));

			const float windowWidth = ImGui::GetWindowSize().x;
			const float textWidth = ImGui::CalcTextSize(pageText.c_str()).x;
			const float navButtonWidth = ImGui::CalcTextSize(">").x + ImGui::GetStyle().FramePadding.x * 2;
			const float buttonWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2;

			const float centerX = (windowWidth - textWidth) * 0.5f;
			ImGui::SetCursorPosX(centerX);
			ImGui::Text(pageText.c_str());
			ImGui::SameLine();

			ImGui::SetCursorPosX(windowWidth - buttonWidth - ImGui::GetStyle().WindowPadding.x * 2.0f);
			if (ImGui::Button("X"))
			{
				helpMenuVisibility = false;
			}

			ImGui::SetCursorPosY(windowWidth * 0.055f - ImGui::GetStyle().WindowPadding.y * 1.2f);
			if (ImGui::Button("<"))
			{
				PreviousPage();
			}

			ImGui::SameLine();
			ImGui::SetCursorPosX(windowWidth - navButtonWidth - ImGui::GetStyle().WindowPadding.x * 2.0f);
			ImGui::SetCursorPosY(windowWidth * 0.055f - ImGui::GetStyle().WindowPadding.y * 1.2f);
			if (ImGui::Button(">"))
			{
				NextPage();
			}

			switch (currentHelperPage)
			{
			case PageType::GLOSSARY_PAGE:
			{
				Utils::UI::DisplayGlossaryPage();
				break;
			}
			case PageType::LOAD_STORE_PAGE:
			case PageType::ARITHMETIC_PAGE:
			case PageType::LOGICAL_PAGE:
			case PageType::COMPARE_PAGE:
			case PageType::JUMP_PAGE:
			case PageType::STACK_PAGE:
			{
				Utils::UI::DisplayTable(INSTRUCTION_TABLES[
					static_cast<uint8_t>(currentHelperPage) -
					static_cast<uint8_t>(PageType::LOAD_STORE_PAGE)
				]);
				break;
			}
			case PageType::MISC_PAGE:
			{
				Utils::UI::DisplayMiscPage("Misc Instructions Table", "misc_table", { 0x00, 0x70, 0xF8, 0xF3, 0xFF });
				break;
			}
			}
			ImGui::End();
		}
	}

	void DrawOutput(const Assembler& assembler, CPU& cpu, Terminal& terminal, InterruptController& interruptController,
		Keyboard& keyboard, bool& executeAuto)
	{
		ImGui::Separator();

		if (ImGui::BeginTabBar("##bottomPanel"))
		{
			const auto errorListTabFlag = Utils::UI::ErrorListTab(tab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
			if (ImGui::BeginTabItem("Error List", nullptr, errorListTabFlag))
			{
				if (ImGui::IsItemClicked())
				{
					tab = SelectedTab::ERROR_LIST;
				}

				ImGui::EndTabItem();
			}
			if (ImGui::IsItemClicked())
			{
				tab = SelectedTab::ERROR_LIST;
			}

			const auto consoleTabFlag = Utils::UI::ConsoleListTab(tab) ? ImGuiTabItemFlags_None : ImGuiTabItemFlags_SetSelected;
			if (ImGui::BeginTabItem("Console", nullptr, consoleTabFlag))
			{
				if (ImGui::IsItemClicked())
				{
					tab = SelectedTab::CONSOLE;
				}
				ImGui::EndTabItem();
			}
			if (ImGui::IsItemClicked())
			{
				tab = SelectedTab::CONSOLE;
			}
			ImGui::EndTabBar();

			switch (tab)
			{
			case SelectedTab::ERROR_LIST: { DrawErrorList(assembler.GetErrors(), assembler.GetWarnings()); break; }
			case SelectedTab::CONSOLE: { DrawConsole(cpu, terminal, interruptController, keyboard, executeAuto); break; }
			}
		}
	}

	void DrawErrorList(std::span<const AssemblerError> errors, std::span<const AssemblerWarning> warnings)
	{
		ImGui::BeginChild("##errorList", ImVec2(-1, 150), true);
		if (errors.empty() && warnings.empty())
		{
			ImGui::TextDisabled("No errors.");
		}
		else
		{
			for (const auto& warning : warnings)
			{
				ImGui::TextWrapped("%s", warning.message.c_str());
			}

			for (const auto& error : errors)
			{
				ImGui::TextWrapped("%s", error.message.c_str());
			}
		}
		ImGui::EndChild();
	}

	void DrawConsole(CPU& cpu, Terminal& terminal, InterruptController& interruptController, Keyboard& keyboard,
					bool& executeAuto)
	{
		static char consoleBuffer[CONSOLE_BUFFER_SIZE];
		static bool scrollToBottom = true;

		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec4 consoleBg = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
		const ImVec4 consoleText = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
		const ImVec4 errorText = ImVec4(0.90f, 0.29f, 0.23f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, consoleBg);
		ImGui::BeginChild("##consoleOutput", ImVec2(-1, 200), true);

		ImGui::PushStyleColor(ImGuiCol_Text, consoleText);
		for (const auto& terminalLine : terminal.GetLines())
		{
			if (terminalLine.lineType == LineType::ERROR)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, errorText);
				ImGui::TextWrapped("%s", terminalLine.command.c_str());
				ImGui::PopStyleColor();
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Text, consoleText);
				ImGui::TextWrapped("%s", terminalLine.command.c_str());
				ImGui::PopStyleColor();
			}
			
		}
		ImGui::PopStyleColor();

		if (scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}
		scrollToBottom = false;

		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_FrameBg, consoleBg);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, consoleBg);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, consoleBg);
		ImGui::PushStyleColor(ImGuiCol_Text, consoleText);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

		ImGui::TextUnformatted(">");
		ImGui::SameLine();

		ImGui::SetNextItemWidth(-1);

		const bool reclaimFocus = ImGui::IsWindowAppearing();
		const auto inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;

		if (ImGui::InputText("##consoleInput", consoleBuffer, IM_ARRAYSIZE(consoleBuffer), inputFlags))
		{
			const std::string line{ consoleBuffer };
			if (interruptController.GetInterruptFlag())
			{
				//executeAuto = false;
				interruptController.EnableInterrupts(false);
				interruptController.RaiseInterrupt(KEYBOARD_INTERRUPT_LINE);
				
				for (uint8_t key : line)
				{
					keyboard.AddKey(key);
				}
				cpu.HandleInterrupt();
			}
			else
			{
				terminal.ExecuteCommand(line);
			}

			consoleBuffer[0] = '\0'; // empty buffer
			scrollToBottom = true;
		}

		if (reclaimFocus || (ImGui::IsItemDeactivatedAfterEdit()))
		{
			ImGui::SetKeyboardFocusHere(-1);
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(4);

		ImGui::EndChild();
	}
}