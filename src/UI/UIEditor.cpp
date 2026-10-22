#include "UI/UIEditor.h"

#include "Assembler/Assembler.h"
#include "Core/Emulator.h"

namespace UIEditor
{
	namespace
	{
		template<typename... Args>
		void TextF(std::format_string<Args...> formatted, Args&&... args)
		{
			const std::string s = std::format(formatted, std::forward<Args>(args)...);
			ImGui::TextUnformatted(s.c_str());
		}

		struct GPView
		{
			const char* name = nullptr;
			Reg16 full{};
			Reg8 hi{};
			Reg8 lo{};
		};

		constexpr GPView GP[] = {
			{"AX", Reg16::AX, Reg8::AH, Reg8::AL},
			{"BX", Reg16::BX, Reg8::BH, Reg8::BL},
			{"CX", Reg16::CX, Reg8::CH, Reg8::CL},
			{"DX", Reg16::DX, Reg8::DH, Reg8::DL}
		};

		constexpr const char* segNames[] = { "ES", "CS", "SS", "DS" };
		constexpr float kSecondColumnX = 250.0f;


		SelectedTab tab{ SelectedTab::ERROR_LIST };
		PageType currentHelperPage{ PageType::GLOSSARY_PAGE };

		constexpr uint16_t PAGE_SIZE = 256u;

		constexpr uint16_t FIRST_LOAD_INSTRUCTION = 0x01;
		constexpr uint16_t LAST_LOAD_INSTRUCTION = 0x14;

		constexpr uint16_t FIRST_ARITHMETIC_INSTRUCTION = 0x20;
		constexpr uint16_t LAST_ARITHMETIC_INSTRUCTION = 0x2B;

		constexpr uint16_t FIRST_LOGIC_INSTRUCTION = 0x30;
		constexpr uint16_t LAST_LOGIC_INSTRUCTION = 0x38;

		constexpr uint16_t FIRST_COMPARE_INSTRUCTION = 0x40;
		constexpr uint16_t LAST_COMPARE_INSTRUCTION = 0x41;

		constexpr uint16_t FIRST_JUMP_INSTRUCTION = 0x50;
		constexpr uint16_t LAST_JUMP_INSTRUCTION = 0x58;

		constexpr uint16_t FIRST_STACK_INSTRUCTION = 0x60;
		constexpr uint16_t LAST_STACK_INSTRUCTION = 0x63;

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

		uint16_t currentPage{};

		bool helpMenuVisibility{ false };

		void NextPage()
		{
			if (currentHelperPage != PageType::MISC_PAGE)
			{
				currentHelperPage = static_cast<PageType>(static_cast<uint16_t>(currentHelperPage) + 1);
			}
		}

		void PreviousPage()
		{
			if (currentHelperPage != PageType::GLOSSARY_PAGE)
			{
				currentHelperPage = static_cast<PageType>(static_cast<uint16_t>(currentHelperPage) - 1);
			}
		}
	}

	void DrawCPUState(const CPU& cpu)
	{
		ImGui::SetNextWindowPos(Utils::UI::CalcCPUStatePanelPos(), ImGuiCond_Always);
		ImGui::SetNextWindowSize(Utils::UI::CalcCPUStatePanelSize(), ImGuiCond_Always);
		const auto windowFlags = ImGuiWindowFlags_NoResize |
								ImGuiWindowFlags_NoMove |
								ImGuiWindowFlags_NoCollapse;

		const auto& regFile = cpu.GetRegisterFile();
		const auto& segmentReg = cpu.GetStatusRegister();

		ImGui::Begin("CPU State", nullptr, windowFlags);

		TextF("CS:IP = {:04X}:{:04X} (Physical: {:05X})",
			regFile.CS(), cpu.GetIP(), cpu.PhysAddr().index());
		TextF("Instruction Register: {:04X}", cpu.GetIR());
		ImGui::Separator();

		ImGui::TextUnformatted("General Purpose Registers");
		ImGui::SameLine(kSecondColumnX);
		ImGui::TextUnformatted("Segment Registrers");

		ImGui::Text(std::format("AX: {} | AH: {} | AL: {}", cpu.GetRegisterFile().AX(),
			cpu.GetRegisterFile().Get8(Reg8::AH), cpu.GetRegisterFile().Get8(Reg8::AL)).c_str());
		ImGui::SameLine(250.0f);
		ImGui::Text(std::format("ES: {}", cpu.GetRegisterFile().ES()).c_str());

		ImGui::Text(std::format("BX: {} | BH: {} | BL: {}", cpu.GetRegisterFile().BX(),
			cpu.GetRegisterFile().Get8(Reg8::BH), cpu.GetRegisterFile().Get8(Reg8::BL)).c_str());
		ImGui::SameLine(250.0f);
		ImGui::Text(std::format("CS: {}", cpu.GetRegisterFile().CS()).c_str());

		ImGui::Text(std::format("CX: {} | CH: {} | CL: {}", cpu.GetRegisterFile().CX(),
			cpu.GetRegisterFile().Get8(Reg8::CH), cpu.GetRegisterFile().Get8(Reg8::CL)).c_str());
		ImGui::SameLine(250.0f);
		ImGui::Text(std::format("SS: {}", cpu.GetRegisterFile().SS()).c_str());

		ImGui::Text(std::format("DX: {} | DH: {} | DL: {}", cpu.GetRegisterFile().DX(),
			cpu.GetRegisterFile().Get8(Reg8::DH), cpu.GetRegisterFile().Get8(Reg8::DL)).c_str());
		ImGui::SameLine(250.0f);
		ImGui::Text(std::format("DS: {}", cpu.GetRegisterFile().DS()).c_str());

		ImGui::Separator();

		ImGui::Text("\nFlags");
		ImGui::Text(std::format("Interrupt Flag: {}", cpu.GetInterruptController().GetInterruptFlag() ? "True" : "False").c_str());
		ImGui::Text(std::format("Zero Flag: {}", cpu.GetStatusRegister().Zero() ? "True" : "False").c_str());
		ImGui::Text(std::format("Carry Flag: {}", cpu.GetStatusRegister().Carry() ? "True" : "False").c_str());
		ImGui::Text(std::format("Negative Flag: {}", cpu.GetStatusRegister().Sign() ? "True" : "False").c_str());
		ImGui::Text(std::format("Overflow Flag: {}", cpu.GetStatusRegister().Overflow() ? "True" : "False").c_str());
		ImGui::Text(std::format("Halt Flag: {}", cpu.GetHaltFlag() ? "True" : "False").c_str());
		ImGui::Separator();
		ImGui::End();
	}

	void DrawCodePanel(Emulator& emulator, AppConfig& config)
	{
		static char editorBuffer[BUFFER_SIZE];
		const auto flags = ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoTitleBar;
		ImGui::SetNextWindowPos(Utils::UI::CalcCodePanelPos(), ImGuiCond_Always);
		ImGui::SetNextWindowSize(Utils::UI::CalcCodePanelSize(), ImGuiCond_Always);


		ImGui::Begin("Code Panel", nullptr, flags);

		ImGui::InputTextMultiline
		("##editor", editorBuffer, IM_ARRAYSIZE(editorBuffer), Utils::UI::CalcTextEditorSize(), ImGuiInputTextFlags_AllowTabInput);

		if (ImGui::Button("Assemble & Load"))
		{
			Logger::ClearLogFile();
			//if (emulator.GetToolchain().GetAssembler().Assemble(editorBuffer))
			//{
			//	const auto& statements = emulator.GetToolchain().GetAssembler().GetStatements();
			//	const Program program = DataLoader::ParseStatements(statements, emulator.GetToolchain().GetAssembler().GetStartAddress());

			//	emulator.GetCPU().GetMemoryUnit().LoadValuesIntoMemory(program);
			//	emulator.GetCPU().Reset();

			//	tab = SelectedTab::CONSOLE;
			//}
			//else
			//{
			//	tab = SelectedTab::ERROR_LIST;
			//}
			std::vector<uint8_t> bytes = DataLoader::ParseHexValues(editorBuffer);
			emulator.GetCPU().GetMemoryUnit().LoadBytesIntoMemory(bytes);
			emulator.GetCPU().SetInstructionRegister();
		}
		DrawOutput(emulator, config);
		ImGui::End();
	}

	void DrawCPUDiagram()
	{
		ImGui::SetNextWindowPos(Utils::UI::CalcCPUDiagramPos());
		ImGui::SetNextWindowSize(Utils::UI::CalcCPUDiagramSize());

		ImGui::Begin("CPU Diagram");
		ImGui::Text("cpu diagram");
		ImGui::End();
	}

	void DrawDissasemblyPanel(MemoryUnit& memoryUnit, const CPU& cpu, const Dissasembler& dissasembler)
	{
		ImGui::SetNextWindowPos(Utils::UI::CalcDissasemblyPanelPos(), ImGuiCond_Always);
		ImGui::SetNextWindowSize(Utils::UI::CalcDissasemblyPanelSize(), ImGuiCond_Always);
		const auto flags = ImGuiWindowFlags_NoCollapse |
							ImGuiWindowFlags_NoMove |
							ImGuiWindowFlags_NoResize;

		ImGui::Begin("Dissasembly Panel", nullptr, flags);

		if (memoryUnit.IsMemoryEmpty())
		{
			ImGui::TextDisabled("Memory Unit is empty.");
			ImGui::End();
			return;
		}
		for (size_t index = 0; index < Utils::Constants::MEMORY_SIZE; ++index)
		{
			std::optional<uint16_t> opcode = memoryUnit.Peek(static_cast<uint16_t>(index));
			if (!opcode.has_value())
			{
				continue;
			}

			const InstructionDef& instruction = dissasembler.GetInstructionDef(opcode.value());

			if (index == cpu.GetIP())
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
		ImGui::End();
	}

	void DrawMemoryView(Emulator& emulator, AppConfig& config)
	{
		if (config.followPC)
		{
			currentPage = static_cast<uint16_t>(emulator.GetCPU().GetIP() / PAGE_SIZE);
		}

		ImGui::SetNextWindowPos(Utils::UI::CalcMemoryViewPos(), ImGuiCond_Always);
		ImGui::SetNextWindowSize(Utils::UI::CalcMemoryViewSize(), ImGuiCond_Always);

		const auto memoryFlags = ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse;

		ImGui::Begin(std::format("Memory View (Page {}/{})", currentPage, PAGE_SIZE - 1).c_str(), nullptr, memoryFlags);
		if (ImGui::Button("Prev"))
		{
			currentPage = currentPage - 1 < 0 ? 255 : currentPage - 1;
		}
		ImGui::SameLine();

		if (ImGui::Button("Next"))
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
				if (index == emulator.GetCPU().GetIP())
				{
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 0, 255, 255));
				}

				if (emulator.GetCPU().IsReadingInstruction() && index == emulator.GetCPU().ComputeAddress(emulator.GetCPU().GetIP() + 1))
				{
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 255, 0, 255));
				}
				else if (emulator.GetCPU().IsWritingInstruction() &&
					index == emulator.GetCPU().ComputeAddress(emulator.GetCPU().GetIP() + 1))
				{
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 0, 0, 255));
				}

				char cellText[8];
				if (const auto result = emulator.GetCPU().GetMemoryUnit()[static_cast<uint16_t>(index)]; result.has_value())
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
	void DrawMenu(Emulator& emulator, AppConfig& config)
	{
		ImGui::SetNextWindowPos(Utils::UI::CalcMenuWindowPos(), ImGuiCond_Always);
		ImGui::SetNextWindowSize(Utils::UI::CalcMenuWindowSize(), ImGuiCond_Always);

		const auto menuFlags = ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse;
		ImGui::Begin("Menu", nullptr, menuFlags);
		if (ImGui::Button("\t\tNext Step\t\t"))
		{
			emulator.GetCPU().Step();
		}
		ImGui::SameLine();

		if (ImGui::Button("\t\tReset\t\t"))
		{
			emulator.GetCPU().GetMemoryUnit().Clear();
			emulator.GetCPU().Reset();
			config.executeAuto = false;
			config.followPC = true;
		}
		ImGui::SameLine();

		if (ImGui::Button(std::format("Auto ({})", config.executeAuto ? "ON" : "OFF").c_str()))
		{
			config.executeAuto = !config.executeAuto;
		}
		ImGui::SameLine();

		if (ImGui::Button(std::format("Follow IP ({})", config.followPC ? "ON" : "OFF").c_str()))
		{
			config.followPC = !config.followPC;
		}

		ImGui::SameLine();
		if (ImGui::Button("Help"))
		{
			helpMenuVisibility = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(Utils::UI::CalcSliderWidth());
		ImGui::SliderFloat("Execution Speed", &config.autoSpeed, 0.1f, 2.0f);

		ImGui::End();
	}

	void DrawHelpMenu()
	{
		if (helpMenuVisibility)
		{
			ImGui::Begin("Help Menu");

			const std::string pageText = std::format("Page {}/8", static_cast<uint16_t>(currentHelperPage));

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
					static_cast<uint16_t>(currentHelperPage) -
						static_cast<uint16_t>(PageType::LOAD_STORE_PAGE)
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

	void DrawOutput(Emulator& emulator, AppConfig& config)
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
			case SelectedTab::ERROR_LIST: { DrawErrorList(emulator.GetToolchain().GetAssembler().GetErrors(),
											emulator.GetToolchain().GetAssembler().GetWarnings()); break; }
			case SelectedTab::CONSOLE: { DrawConsole(emulator, config); break; }
			}
		}
	}

	void DrawErrorList(std::span<const AssemblerError> errors, std::span<const AssemblerWarning> warnings)
	{
		ImGui::BeginChild("##errorList", Utils::UI::CalcTabSize(), true);
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

	void DrawConsole(Emulator& emulator, AppConfig& config)
	{
		static char consoleBuffer[CONSOLE_BUFFER_SIZE];

		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec4 consoleBg = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
		const ImVec4 consoleText = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
		const ImVec4 errorText = ImVec4(0.90f, 0.29f, 0.23f, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, consoleBg);
		ImGui::BeginChild("##consoleOutput", Utils::UI::CalcTabSize(), true);

		ImGui::PushStyleColor(ImGuiCol_Text, consoleText);
		for (const auto& terminalLine : emulator.GetTerminal().GetLines())
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
			if (emulator.GetInterruptController().GetInterruptFlag())
			{
				//executeAuto = false;
				emulator.GetInterruptController().EnableInterrupts(false);
				emulator.GetInterruptController().RaiseInterrupt(KEYBOARD_INTERRUPT_LINE);

				for (uint16_t key : line)
				{
					emulator.GetKeyboard().AddKey(key);
				}
				emulator.GetCPU().HandleInterrupt();
			}
			else
			{
				emulator.GetTerminal().ExecuteCommand(line);
			}

			consoleBuffer[0] = '\0'; // empty buffer
		}

		if (reclaimFocus || (ImGui::IsItemDeactivatedAfterEdit()))
		{
			ImGui::SetKeyboardFocusHere(-1);
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(4);

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
		{
			ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
	}
}