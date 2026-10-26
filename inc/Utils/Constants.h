#pragma once

#include <cstdint>

namespace Utils
{
	namespace Constants
	{
		constexpr uint32_t MEMORY_SIZE = 1048576;

		constexpr uint32_t MASK_20_BIT{ 0xFFFFF };

		constexpr uint16_t CF_MASK{ 1u << 0 }; // Carry flag
		constexpr uint16_t PF_MASK{ 1u << 2 }; // Parity flag
		constexpr uint16_t AF_MASK{ 1u << 4 }; // Aux carry flag
		constexpr uint16_t ZF_MASK{ 1u << 6 }; // Zero flag
		constexpr uint16_t SF_MASK{ 1u << 7 }; // Sign flag

		constexpr uint16_t TF_MASK{ 1u << 8 }; // Trap flag
		constexpr uint16_t IF_MASK{ 1u << 9 }; // Interrupt flag
		constexpr uint16_t DF_MASK{ 1u << 10 }; // Directional flag

		constexpr uint16_t OF_MASK{ 1u << 11 }; // Overflow flag

		constexpr uint16_t STATUS_MASK{ CF_MASK | PF_MASK | AF_MASK | ZF_MASK | SF_MASK | OF_MASK };
		constexpr uint16_t CONTROL_MASK{ TF_MASK | IF_MASK | DF_MASK };

		constexpr uint16_t FLAGS_DEFAULT_VALUE{ 0b0000000000101010 };

		constexpr float MENU_WINDOW_WIDTH_PROCENT{ 0.6f };
		constexpr float MENU_WINDOW_HEIGHT_PROCENT{ 0.05f };

		constexpr float MEMORY_VIEW_WINDOW_WIDTH_PROCENT{ 0.6f };
		constexpr float MEMORY_VIEW_WINDOW_HEIGHT_PROCENT{ 0.24f };

		constexpr float CODE_PANEL_WINDOW_WIDTH_PROCENT{ 0.6f };
		constexpr float CODE_PANEL_WINDOW_HEIGHT_PROCENT{ 0.71f };

		constexpr float TEXT_EDITOR_WINDOW_HEIGHT_PROCENT{ 0.4861f };
		constexpr float TAB_HEIGHT_PROCENT{ 0.1215f };

		constexpr float CPU_STATE_WINDOW_WIDTH_PROCENT{ 0.2f };
		constexpr float CPU_STATE_WINDOW_HEIGHT_PROCENT{ 0.29f };

		constexpr float CPU_DIAGRAM_WIDTH_PROCENT{ 0.4f };
		constexpr float CPU_DIAGRAM_HEIGHT_PROCENT{ 0.4f };

		constexpr float DISSASEMBLY_WINDOW_WIDTH_PROCENT{ 0.2f };
		constexpr float DISSASEMBLY_WINDOW_HEIGHT_PROCENT{ 0.29f };

		constexpr float SLIDER_WIDTH_PROCENT{ 0.058f };
	}
}