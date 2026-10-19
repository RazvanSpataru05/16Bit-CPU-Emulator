#include "Utils/UIUtils.h"

namespace Utils
{
	namespace
	{
		constexpr std::pair<const char*, const char*> glossary[] =
		{
			{"PC", "Program Counter"},
			{"IR", "Instruction Register"},
			{"SP", "Stack Pointer"},
			{"A", "Accumulator Register"},
			{"B", ""},
			{"C", "Counter Register"},
			{"D", ""}
		};

		constexpr uint16_t HELP_TABLE_COLUMN_SIZE = 5u;

		void SetupInstructionsTableColumn()
		{
			ImGui::TableSetupColumn("Mnemonic", ImGuiTableColumnFlags_WidthFixed, 92.0f);
			ImGui::TableSetupColumn("Opcode", ImGuiTableColumnFlags_WidthFixed, 56.0f);
			ImGui::TableSetupColumn("Operator Kind", ImGuiTableColumnFlags_WidthFixed, 200.0f);
			ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 36.0f);
			ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);

			ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, ImVec4(0.18f, 0.28f, 0.45f, 1.0f));
			ImGui::TableHeadersRow();
			ImGui::PopStyleColor();
		}

		void TextCentered(std::string_view text)
		{
			const float cellWidth = ImGui::GetColumnWidth();
			const float textWidth = ImGui::CalcTextSize(text.data()).x;
			const float offset = (cellWidth - textWidth) * 0.5f;

			if (offset > 0.0f)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
			}
			ImGui::TextUnformatted(text.data(), text.data() + text.size());
		}

		void DisplayMnemonic(size_t index)
		{
			int column = 0;
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(column++);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.80f, 1.00f, 1.0f));
			TextCentered(ISA::Table[index].mnemonic);
			ImGui::PopStyleColor();

			ImGui::TableSetColumnIndex(column++);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 0.75f, 0.20f, 1.0f));
			TextCentered(std::format("0x{:02X}", ISA::Table[index].opcode));
			ImGui::PopStyleColor();

			ImGui::TableSetColumnIndex(column++);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.40f, 0.90f, 1.0f));
			TextCentered(Utils::String::OperatorKindToString(ISA::Table[index].operatorKind));
			ImGui::PopStyleColor();

			ImGui::TableSetColumnIndex(column++);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.95f, 0.55f, 1.0f));
			TextCentered(std::format("{}", ISA::Table[index].size));
			ImGui::PopStyleColor();

			ImGui::TableSetColumnIndex(column++);
			ImGui::TextWrapped("%s", ISA::Table[index].description);
		}

		void DisplayMnemonicList(std::initializer_list<uint16_t> opcodes)
		{
			SetupInstructionsTableColumn();

			const size_t tableSize = ISA::GetISATableSize();
			for (size_t index = 0; index < tableSize; ++index)
			{
				for (uint16_t opcode : opcodes)
				{
					if (ISA::Table[index].opcode == opcode)
					{
						DisplayMnemonic(index);
					}
				}
			}
		}

		void DisplayPageInstructions(uint16_t firstInstruction, uint16_t lastInstruction)
		{
			SetupInstructionsTableColumn();

			const size_t tableSize = ISA::GetISATableSize();
			for (size_t index = 0; index < tableSize; ++index)
			{
				if (ISA::Table[index].opcode >= firstInstruction
					&& ISA::Table[index].opcode <= lastInstruction)
				{
					DisplayMnemonic(index);
				}

				if (ISA::Table[index].opcode > lastInstruction)
				{
					return;
				}
			}
		}
	}

	namespace UI
	{
		bool ErrorListTab(SelectedTab tab)
		{
			return tab == SelectedTab::ERROR_LIST;
		}

		bool ConsoleListTab(SelectedTab tab)
		{
			return tab == SelectedTab::CONSOLE;
		}

		void DisplayInstruction(const InstructionDef& instruction, MemoryUnit& memoryUnit, uint16_t index, std::optional<uint16_t> opcode)
		{
			if (!opcode.has_value())
			{
				ImGui::Text("--");
				return;
			}

			const MemoryCellType cellType = memoryUnit.GetMemoryCellType(index);
			if (cellType == MemoryCellType::UNKNOWN || cellType == MemoryCellType::DATA)
			{
				return;
			}

			switch (instruction.size)
			{
			case 1u:
			{
				ImGui::Text(std::format("{:#04x}: {}", opcode.value(), instruction.mnemonic).c_str());
				break;
			}
			case 2u:
			{
				uint16_t secondByte = memoryUnit[index + 1].value();
				ImGui::Text(std::format("{:#04x}: {} {}", opcode.value(), instruction.mnemonic, secondByte).c_str());
				break;
			}
			case 3u:
			{
				uint16_t secondByte = memoryUnit[index + 1].value();
				uint16_t thirdByte = memoryUnit[index + 2].value();
				uint16_t address = (secondByte << 8) | thirdByte;
				ImGui::Text(std::format("{:#04x}: {} {:#06X}", opcode.value(), instruction.mnemonic, address).c_str());
				break;
			}
			default:
				ImGui::Text(std::format("{:#06x}: ???", index).c_str());
				break;
			}
		}

		void DisplayGlossaryPage()
		{
			const char* title = "Glossary";
			const float titleWidth = ImGui::CalcTextSize(title).x;
			const float windowWidth = ImGui::GetWindowSize().x;

			ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
			ImGui::SetCursorPosY(windowWidth * 0.05f);
			ImGui::Text(title);
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::BeginTable("##glossary_table", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Definition", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();

				for (const auto& [mnemonic, definition] : glossary)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), mnemonic);
					ImGui::TableSetColumnIndex(1);
					ImGui::TextWrapped(definition);
				}
				ImGui::EndTable();
			}
		}

		void DisplayMiscPage(const char* title, const char* tableId, std::initializer_list<uint16_t> opcodes)
		{
			const float titleWidth = ImGui::CalcTextSize(title).x;
			const float windowWidth = ImGui::GetWindowSize().x;

			ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
			ImGui::SetCursorPosY(windowWidth * 0.05f);
			ImGui::TextColored(ImVec4(0.90f, 0.90f, 0.90f, 1.0f), title);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.35f, 0.55f, 0.85f, 1.0f));
			ImGui::Separator();
			ImGui::PopStyleColor();
			ImGui::Spacing();

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 5.0f));

			if (ImGui::BeginTable(tableId, HELP_TABLE_COLUMN_SIZE,
				ImGuiTableFlags_BordersInnerV |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_PadOuterX))
			{
				DisplayMnemonicList(opcodes);
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
		}

		void DisplayTable(const InstructionTableInfo& info)
		{
			const float windowWidth = ImGui::GetWindowSize().x;
			const float titleWidth = ImGui::CalcTextSize(info.title).x;

			ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
			ImGui::SetCursorPosY(windowWidth * 0.05f);
			ImGui::TextColored(ImVec4(0.90f, 0.90f, 0.90f, 1.0f), info.title);
			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.35f, 0.55f, 0.85f, 1.0f));
			ImGui::Separator();
			ImGui::PopStyleColor();
			ImGui::Spacing();

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 5.0f));
			if (ImGui::BeginTable(info.tableId, HELP_TABLE_COLUMN_SIZE,
				ImGuiTableFlags_BordersInnerV |
				ImGuiTableFlags_RowBg |
				ImGuiTableFlags_PadOuterX))
			{
				DisplayPageInstructions(info.firstInstruction, info.lastInstruction);
				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
		}

		ImVec2 CalcMenuWindowPos()
		{
			return ImVec2(0.0f, 0.0f);
		}

		ImVec2 CalcMenuWindowSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowX = io.DisplaySize.x;
			const float windowY = io.DisplaySize.y;

			return ImVec2(windowX * Utils::Constants::MENU_WINDOW_WIDTH_PROCENT,
				windowY * Utils::Constants::MENU_WINDOW_HEIGHT_PROCENT);
		}

		ImVec2 CalcMemoryViewPos()
		{
			const ImVec2 menuSize = CalcMenuWindowSize();
			return ImVec2(0.0f, menuSize.y);
		}

		ImVec2 CalcMemoryViewSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowX = io.DisplaySize.x;
			const float windowY = io.DisplaySize.y;

			return ImVec2(windowX * Utils::Constants::MEMORY_VIEW_WINDOW_WIDTH_PROCENT,
				windowY * Utils::Constants::MEMORY_VIEW_WINDOW_HEIGHT_PROCENT);
		}

		ImVec2 CalcCodePanelPos()
		{
			const ImVec2 memorySize = CalcMemoryViewSize();
			const ImVec2 memoryPos = CalcMemoryViewPos();
			return ImVec2(0.0f, memoryPos.y + memorySize.y);
		}

		ImVec2 CalcCodePanelSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowX = io.DisplaySize.x;
			const float windowY = io.DisplaySize.y;

			return ImVec2(windowX * Utils::Constants::CODE_PANEL_WINDOW_WIDTH_PROCENT, 
				windowY * Utils::Constants::CODE_PANEL_WINDOW_HEIGHT_PROCENT);
		}

		ImVec2 CalcDissasemblyPanelPos()
		{
			const ImVec2 menuSize = CalcMenuWindowSize();

			return ImVec2(menuSize.x, 0.0f);
		}

		ImVec2 CalcDissasemblyPanelSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowX = io.DisplaySize.x;
			const float windowY = io.DisplaySize.y;

			return ImVec2(windowX * Utils::Constants::DISSASEMBLY_WINDOW_WIDTH_PROCENT, 
						windowY * Utils::Constants::DISSASEMBLY_WINDOW_HEIGHT_PROCENT);
		}

		ImVec2 CalcCPUStatePanelPos()
		{
			const ImVec2 menuSize = CalcMenuWindowSize();
			const ImVec2 dissasemblySize = CalcDissasemblyPanelSize();

			return ImVec2(menuSize.x + dissasemblySize.x, 0.0f);
		}

		ImVec2 CalcCPUStatePanelSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowX = io.DisplaySize.x;
			const float windowY = io.DisplaySize.y;

			return ImVec2(windowX * Utils::Constants::CPU_STATE_WINDOW_WIDTH_PROCENT,
				windowY * Utils::Constants::CPU_STATE_WINDOW_HEIGHT_PROCENT);
		}

		ImVec2 CalcTextEditorSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowY = io.DisplaySize.y;

			return ImVec2(-1, windowY * Utils::Constants::TEXT_EDITOR_WINDOW_HEIGHT_PROCENT);
		}

		ImVec2 CalcTabSize()
		{
			ImGuiIO& io = ImGui::GetIO();
			const float windowY = io.DisplaySize.y;

			return ImVec2(-1, windowY * Utils::Constants::TAB_HEIGHT_PROCENT);
		}
	}
}
