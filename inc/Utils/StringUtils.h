#pragma once

#include "Utils/CharacterUtils.h"

#include "UI/UIEditor.h"

#include "Assembler/ISAEntry.h"
#include "Assembler/Token.h"
#include "Assembler/ParsedNumber.h"
#include "Assembler/Slot.h"

#include <string>
#include <algorithm>
#include <format>

namespace Utils
{
	namespace String
	{
		std::string ToUpper(std::string_view word);
		std::string ToLower(std::string_view word);
		std::string RemoveWhiteSpace(std::string_view word);

		void CheckBase(std::string_view prefix, uint16_t& base);

		bool HasPrefix(std::string_view word);
		bool StartsLikeNumber(std::string_view word);
		bool IsNumber(std::string_view word);
		ParsedNumber ParseNumber(const std::string& word);

		std::string_view	OperatorKindToString(ISA::OperandKind operatorKind);
		std::string_view	SlotToString(Slot slot);
		std::string			TokenTypeToString(const Token& token);
		
		template<typename... Args>
		std::string TextFormat(std::format_string<Args...> format, Args&&... args)
		{
			const std::string s = std::format(format, std::forward<Args>(args)...);
			return s;
		}
	}

};  