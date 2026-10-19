#pragma once

#include <cstdint>

namespace Utils
{
	namespace Constants
	{
		constexpr uint32_t MEMORY_SIZE = 1048576;

		constexpr uint32_t MASK_20_BIT{ 0xFFFFF };

		constexpr float MENU_WINDOW_WIDTH_PROCENT{ 0.6f };
		constexpr float MENU_WINDOW_HEIGHT_PROCENT{ 0.05f };

		constexpr float MEMORY_VIEW_WINDOW_WIDTH_PROCENT{ 0.6f };
		constexpr float MEMORY_VIEW_WINDOW_HEIGHT_PROCENT{ 0.24f };

		constexpr float CODE_PANEL_WINDOW_WIDTH_PROCENT{ 0.6f };
		constexpr float CODE_PANEL_WINDOW_HEIGHT_PROCENT{ 0.71f };

		constexpr float TEXT_EDITOR_WINDOW_HEIGHT_PROCENT{ 0.4861f };
		constexpr float TAB_HEIGHT_PROCENT{ 0.1215f };
	}
}