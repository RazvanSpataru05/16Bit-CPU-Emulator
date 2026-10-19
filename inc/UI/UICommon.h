#pragma once

#include <cstdint>

namespace UIEditor
{
	enum class SelectedTab : uint8_t
	{
		NONE = 0u,
		ERROR_LIST,
		CONSOLE
	};

	enum class Mode : uint8_t
	{
		None = 0u,
		EDIT,
		DISSASEMBLY
	};

	enum class PageType : uint8_t
	{
		NONE = 0u,
		GLOSSARY_PAGE,
		LOAD_STORE_PAGE,
		ARITHMETIC_PAGE,
		LOGICAL_PAGE,
		COMPARE_PAGE,
		JUMP_PAGE,
		STACK_PAGE,
		MISC_PAGE
	};
}

