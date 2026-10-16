#include "Utils/StringUtils.h"

namespace Utils
{
	namespace String
	{
		std::string ToUpper(std::string_view word)
		{
			std::string upper{ word };
			std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
			return upper;
		}

		std::string ToLower(std::string_view word)
		{
			std::string lower{ word };
			std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
			return lower;
		}

		std::string RemoveWhiteSpace(std::string_view word)
		{
			std::string s{ word };
			std::remove_if(s.begin(), s.end(), [](unsigned char c) {
				return c == ' ';
				});
			return s;
		}

		void CheckBase(std::string_view prefix, uint16_t& base)
		{
			if (prefix == "0b" || prefix == "0B") base = 2u;
			else if (prefix == "0x" || prefix == "0X") base = 16u;
		}

		bool HasPrefix(std::string_view word)
		{
			if (word.size() < 2) return false;
			return word[0] == '0' && (word[1] == 'b' || word[1] == 'B' || word[1] == 'x' || word[1] == 'X');
		}

		bool StartsLikeNumber(std::string_view word)
		{
			if (word.empty()) return false;
			return HasPrefix(word) || isdigit(word[0]);
		}

		bool IsNumber(std::string_view word)
		{
			std::string prefix;
			uint16_t base = 10u;
			size_t startingPosition{};

			if (HasPrefix(word))
			{
				startingPosition += 2;
				prefix = word.substr(0, 2);
				CheckBase(prefix, base);
			}

			return std::all_of(word.begin() + startingPosition, word.end(), [base](unsigned char c) {
				return Utils::Character::IsValidDigit(c, base);
				});
		}

		ParsedNumber ParseNumber(const std::string& word)
		{
			uint16_t base = 10u;
			std::string_view digits = word;

			if (HasPrefix(word))
			{
				const std::string prefix = word.substr(0, 2);
				CheckBase(prefix, base);
				digits = std::string_view(word).substr(2);
			}

			uint32_t value{};
			auto [ptr, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), value, base);

			if (ec == std::errc::result_out_of_range)
			{
				return { 0u, true };
			}

			return { value, false };
		}

		std::string_view OperatorKindToString(ISA::OperatorKind operatorKind)
		{
			switch (operatorKind)
			{
			case ISA::OperatorKind::NONE:					return "None";
			case ISA::OperatorKind::IMM_8:					return "Immediate 8-bit value";
			case ISA::OperatorKind::ADDR_16:				return "16-bit Address";
			case ISA::OperatorKind::REG:					return "Registry or Selector Code";
			case ISA::OperatorKind::REG_REG:				return "Two Registries";
			}
			return "Error";
		}

		std::string TokenTypeToString(const Token& token)
		{
			switch (token.type)
			{
			case TokenType::REGISTER:		return "register";
			case TokenType::MNEMONIC:		return "mnemonic";
			case TokenType::IDENTIFIER:		return "identifier";
			case TokenType::NUMBER:			return "number";
			case TokenType::NEW_LINE:		return "newline";

			case TokenType::LEFT_PARAN:		return "left paranthesis";
			case TokenType::RIGHT_PARAN:	return "right paranthesis";
			case TokenType::LEFT_BRACKET:	return "left bracket";
			case TokenType::RIGHT_BRACKET:	return "right bracket";

			case TokenType::COLON:			return "colon";
			case TokenType::COMMA:			return "comma";
			case TokenType::DOT:			return "dot";

			case TokenType::DW:				return "DW";

			case TokenType::END_OF_FILE:	return "end of file";

			default:						return "error";
			}
		}
	}
}
