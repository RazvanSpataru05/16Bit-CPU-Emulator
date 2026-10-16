#pragma once

#include "Utils/CharacterUtils.h"

#include "UI/UIEditor.h"

#include "Assembler/ISAEntry.h"
#include "Assembler/Token.h"
#include "Assembler/ParsedNumber.h"

#include <string>
#include <algorithm>

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

		std::string_view OperatorKindToString(ISA::OperatorKind operatorKind);
		std::string TokenTypeToString(const Token& token);
	}

};  