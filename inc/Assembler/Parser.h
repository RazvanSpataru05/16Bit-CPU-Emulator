#pragma once

#include "Assembler/Lexer.h"
#include "Assembler/LabelInfo.h"
#include "Assembler/Statement.h"
#include "Assembler/ParsedNumber.h"

#include "Utils/Logger.h"

#include <span>

using namespace ISA;

using StatementHandler = std::function<void()>;

class Parser
{
public:
	Parser() = default;
	Parser(std::span<const Token> tokens);

	Parser(Parser&&) = default;
	Parser& operator=(Parser&&) = default;

	void AddLabels();
	void ParseTokens();

	bool ParserErrors() const noexcept;

	uint16_t GetStartAddress()									const noexcept;

	std::span<const Statement>			GetStatements()			const noexcept;
	std::span<const AssemblerError>		GetErrors()				const noexcept;
	std::span<const AssemblerWarning>	GetWarnings()			const noexcept;

	void SetTokens(std::span<const Token> tokens);

	void ResetStatement();
	void PrintStatements() const noexcept;
	void PrintLabels() const noexcept;

private:
	Parser(const Parser&) = delete;
	Parser& operator=(const Parser&) = delete;

private:

	// first pass handlers
	void HandleMnemonicToken_Scan();
	void HandleIdentifierToken_Scan();
	void HandleNewLineToken();
	void HandleDotToken();
	void HandleDWToken_Scan();

	// second pass handlers
	void HandleMnemonicToken_Emit();
	void HandleDWToken_Emit();
	void HandleIdentifierToken_Emit();

	// directive handlers
	void HandleOrgDirective();
	void HandleStartDirective();

	std::array<Operand, 2> ConsumeImm8();
	std::array<Operand, 2> ConsumeAddr16();
	std::array<Operand, 2> ConsumeReg();
	std::array<Operand, 2> ConsumeRegReg();

	void AddStatement();
	void AddError(std::string_view message);
	void AddWarning(std::string_view message);

	void ExpectEndOfStatement();
	void ExpectComma();
	void ExpectColon();

	void CheckUnusedLabels();
	void ConsumeLabel();
	bool IsLabelDefinition() const;

	/* Helpers */
	const Token& Peek() const;
	const Token& Next();
	ParsedNumber ConsumeNumber();
	uint16_t ConsumeSelector();
	void ConsumeLine();

	template <typename T>
	bool CheckNumericLimit(const ParsedNumber& parsedNumber);

private:
	std::vector<Token> m_tokens;
	std::vector<AssemblerError> m_errors;
	std::vector<AssemblerWarning> m_warnings;
	std::unordered_map<std::string, LabelInfo> m_labels;

	uint16_t m_startAddress{ 0x0000 };
	uint16_t m_currentAddress{ 0x0000 };
	size_t m_pos{};

	bool m_currentStatementHasError{ false };
	bool m_seenHLT{ false };
	bool m_seenStartDirective{ false };

	Statement m_currentStatement;
	std::vector<Statement> m_statements;

	const std::vector<std::pair<std::function<bool(const Token&)>, StatementHandler>> m_scanHandlers =
	{
		{[](const Token& token) {return token.type == TokenType::MNEMONIC;},
			[this]() {HandleMnemonicToken_Scan();}},

		{[this](const Token& token) {return token.type == TokenType::IDENTIFIER;},
			[this]() {HandleIdentifierToken_Scan();}},

		{[](const Token& token) {return token.type == TokenType::NEW_LINE;},
			[this]() {HandleNewLineToken();}},

		{[](const Token& token) {return token.type == TokenType::DOT;},
			[this]() {HandleDotToken();}},

		{[](const Token& token) {return token.type == TokenType::DW;},
			[this]() {HandleDWToken_Scan();}}
	};

	const std::vector<std::pair<std::function<bool(const Token&)>, StatementHandler>> m_emitHandlers =
	{
		{[](const Token& token) {return token.type == TokenType::MNEMONIC;},
			[this]() {HandleMnemonicToken_Emit();}},

		{[this](const Token& token) {return token.type == TokenType::IDENTIFIER;},
			[this]() {HandleIdentifierToken_Emit();}},

		{[](const Token& token) {return token.type == TokenType::NEW_LINE;},
			[this]() {HandleNewLineToken();}},

		{[](const Token& token) {return token.type == TokenType::DOT;},
			[this]() {HandleDotToken();}},

		{[](const Token& token) {return token.type == TokenType::DW;},
			[this]() {HandleDWToken_Emit();}}
	};

	const std::unordered_map<std::string_view, std::function<void()>> m_directiveHandlers = {
		{"org", [this]() {HandleOrgDirective();}},
		{"start", [this]() {HandleStartDirective();}}
	};
};

template<typename T>
inline bool Parser::CheckNumericLimit(const ParsedNumber& parsedNumber)
{
	if (parsedNumber.outOfRange)
	{
		AddError(std::format("Error at line {}, column {}: '{}' is out of range for a numeric literal (maximum representable value is 0xFFFFFFFF).",
			Peek().line, Peek().column, Peek().value));
		return false;
	}

	const std::string size = std::is_same_v<T, uint16_t> ? "8" : "16";
	if (parsedNumber.value > std::numeric_limits<T>::max())
	{
		AddWarning(std::format("Warning at line {}, column {}: value '{}' exceeds {} bits and will be truncated.",
			Peek().line, Peek().column, Peek().value, size));
	}
	return true;
}
