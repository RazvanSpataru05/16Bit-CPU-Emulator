#pragma once

#include "Assembler/AssemblerError.h"
#include "Assembler/Parser.h"
#include "Assembler/Lexer.h"

#include <span>

class Assembler
{
public:
	Assembler();

	bool Assemble(std::string_view sourceCode);

	bool HasErrors() const noexcept;

	uint16_t GetStartAddress() const noexcept;

	[[nodiscard]] std::span<const Statement>		GetStatements()			const noexcept;
	[[nodiscard]] std::span<const AssemblerError>	GetErrors()				const noexcept;
	[[nodiscard]] std::span<const AssemblerWarning> GetWarnings()			const noexcept;

	void SortErrors()	noexcept;
	void SortWarnings() noexcept;

	static std::vector<uint8_t> Encode(const Statement& statement);

private:
	Assembler(const Assembler&) = delete;
	Assembler& operator=(const Assembler&) = delete;
	Assembler(Assembler&&) = delete;
	Assembler& operator=(Assembler&&) = delete;

private:
	void RunLexer(std::string_view sourceCode);
	void RunParser();

private:
	Lexer m_lexer;
	Parser m_parser;

	std::vector<AssemblerError> m_errors;
	std::vector<AssemblerWarning> m_warnings;
};