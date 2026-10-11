#include "Assembler/Parser.h"

using namespace ISA;

Parser::Parser(std::span<const Token> tokens) :
	m_pos{ 0 },
	m_currentAddress{ 0x0000 },
	m_startAddress{ 0x0000 }
{
	m_tokens.assign(tokens.begin(), tokens.end());
	ResetStatement();
}

bool Parser::ParserErrors() const noexcept
{
	return !m_errors.empty();
}

uint16_t Parser::GetStartAddress() const noexcept
{
	return m_startAddress;
}

std::span<const Statement> Parser::GetStatements() const noexcept
{
	return m_statements;
}

std::span<const AssemblerError> Parser::GetErrors() const noexcept
{
	return m_errors;
}

std::span<const AssemblerWarning> Parser::GetWarnings() const noexcept
{
	return m_warnings;
}

void Parser::SetTokens(std::span<const Token> tokens)
{
	m_tokens.assign(tokens.begin(), tokens.end());
}

void Parser::AddLabels()
{
	m_labels.clear();
	m_pos = 0;
	m_currentAddress = 0x0000;
	m_currentStatementHasError = false;

	Logger::AddInfoMessage("In First Pass\n");
	while (m_pos < m_tokens.size() && Peek().type != TokenType::END_OF_FILE)
	{

		const Token& currentToken = Peek();
		auto it = std::find_if(m_scanHandlers.begin(), m_scanHandlers.end(), [currentToken](const auto& handler) {
			return handler.first(currentToken);
			});

		if (it != m_scanHandlers.end())
		{
			it->second();
		}
		else
		{
			AddError(std::format("Error at line {}, column {}: Undefined symbol '{}'.",
				Peek().line, Peek().column, Peek().value));
			ConsumeLine();
		}
	}
	Logger::AddInfoMessage("Out First Pass\n");
}

void Parser::ParseTokens()
{
	AddLabels();
	PrintLabels();

	m_statements.clear();
	m_errors.clear();
	m_warnings.clear();

	m_pos = 0;
	m_currentAddress = 0x0000;
	m_startAddress = 0x0000;
	m_currentStatementHasError = false;
	m_seenHLT = false;
	m_seenStartDirective = false;
	ResetStatement();

	Logger::AddInfoMessage("In Second Pass\n");
	while (Peek().type != TokenType::END_OF_FILE)
	{
		const Token& currentToken = Peek();

		auto it = std::find_if(m_emitHandlers.begin(), m_emitHandlers.end(), [currentToken](const auto& handler) {
			return handler.first(currentToken);
			});
		if (it != m_emitHandlers.end())
		{
			it->second();
		}
		else
		{
			AddError(std::format("Error at line {}, column {}: '{}' is not a recognized instruction.",
				Peek().line, Peek().column, Peek().value));
			ConsumeLine();
		}
	}
	Logger::AddInfoMessage("Out Second Pass\n");

	CheckUnusedLabels();
	if (!m_seenHLT)
	{
		AddWarning(std::format("Warning: no 'HLT' instruction found anywhere in the program."));
	}
	if (!m_seenStartDirective)
	{
		AddWarning(std::format("Warning: no '.start' directive found anywhere in the program. PC value will have default value 0x0000."));
	}
}

void Parser::AddStatement()
{
	m_statements.emplace_back(m_currentStatement);
}

void Parser::AddError(std::string_view message)
{
	m_errors.emplace_back(AssemblerStage::PARSER, Peek().line, Peek().column, message);
}

void Parser::AddWarning(std::string_view message)
{
	m_warnings.emplace_back(AssemblerStage::PARSER, Peek().line, Peek().column, message);
}

void Parser::ResetStatement()
{
	m_currentStatement.opcode = std::nullopt;
	m_currentStatement.address = 0x0000;
	m_currentStatement.operatorCount = 0u;
	m_currentStatement.operands.fill(0x00);
	m_currentStatement.ISAEntry = nullptr;
}

void Parser::PrintStatements() const noexcept
{
	for (const auto& statement : m_statements)
	{
		if (statement.ISAEntry)
		{
			Logger::AddInfoMessage(std::format("MNEMONIC: {}\n", statement.ISAEntry->mnemonic));
		}

		if (statement.opcode.has_value())
		{
			Logger::AddInfoMessage(std::format("OPCODE: 0x{}\n", static_cast<int>(statement.opcode.value())));
		}
		Logger::AddInfoMessage("VALUE(S): ");

		for (size_t index = 0; index < statement.operatorCount; ++index)
		{
			Logger::AddInfoMessage(std::format("0x{}, ", static_cast<int>(statement.operands[index])));
		}

		Logger::AddInfoMessage("\n\n");
	}
}

void Parser::PrintLabels() const noexcept
{
	Logger::AddInfoMessage(std::format("Labels: {}\n\n", m_labels.size()));
	for (auto it = m_labels.begin(); it != m_labels.end(); it++)
	{
		Logger::AddInfoMessage(std::format("Label: {}\n", it->first));
		Logger::AddInfoMessage(std::format("Address: {}\n", static_cast<int>(it->second.address)));
		Logger::AddInfoMessage(std::format("Line declaration: {}\n", static_cast<int>(it->second.lineDeclaration)));
	}
	Logger::AddInfoMessage("\n\n\n");
}

void Parser::HandleMnemonicToken_Emit()
{
	const ISA::ISAEntry* entry = ISA::Find(Utils::String::ToUpper(Peek().value));
	if (!entry) return;

	m_currentStatement.opcode = entry->opcode;
	m_currentStatement.ISAEntry = entry;
	m_currentStatement.operatorCount = entry->size - 1;
	m_currentStatement.address = m_currentAddress;

	m_currentAddress += entry->size;

	if (Utils::String::ToUpper(Peek().value) == "HLT")
	{
		m_seenHLT = true;
	}

	switch (entry->operatorKind)
	{
	case OperatorKind::NONE: { Next(); break; }
	case OperatorKind::IMM_8: { m_currentStatement.operands = ConsumeImm8();		break; }
	case OperatorKind::ADDR_16: { m_currentStatement.operands = ConsumeAddr16();	break; }
	case OperatorKind::REG: { m_currentStatement.operands = ConsumeReg();			break; }
	case OperatorKind::REG_REG: { m_currentStatement.operands = ConsumeRegReg();	break; }
	default: break;
	}

	ExpectEndOfStatement();
	AddStatement();
}

void Parser::HandleDWToken_Emit()
{
	Next();
	if (Peek().type == TokenType::IDENTIFIER)
	{
		const std::string label = Utils::String::ToLower(Peek().value);
		if (m_labels.find(label) == m_labels.end())
		{
			AddError(std::format("Error at line {}, column {}: Undefined label '{}'.",
				Peek().line, Peek().column, Peek().value));
			ConsumeLine();
			return;
		}

		Logger::AddInfoMessage("Define Word Emit (DWE)\n");
		const uint16_t address = m_labels.at(label).address;

		m_currentStatement.address = m_currentAddress;
		m_currentStatement.operatorCount = static_cast<uint8_t>(m_currentStatement.operands.size()); // the address of the word
		m_currentStatement.operands[0] = (address >> 8) & 0xFF; // hi
		m_currentStatement.operands[1] = address & 0xFF; // lo
		AddStatement();
	}
	else if (Peek().type == TokenType::NUMBER)
	{
		const ParsedNumber parsedNumber = ConsumeNumber();
		if (CheckNumericLimit<uint16_t>(parsedNumber))
		{
			const uint16_t address = static_cast<uint16_t>(parsedNumber.value);

			m_currentStatement.address = address;
			m_currentStatement.operands[0] = (address >> 8) & 0xFF; // hi byte
			m_currentStatement.operands[1] = address & 0xFF; // lo byte
			AddStatement();
		}
	}
	else
	{
		AddError(std::format("Error at line {}, column {}: Expected identifier or 16-bit address, found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
		return;
	}
}

void Parser::HandleMnemonicToken_Scan()
{
	const ISAEntry* entry = ISA::Find(Utils::String::ToUpper(Peek().value));
	if (!entry)
	{
		ConsumeLine();
		return;
	}

	m_currentAddress += entry->size;
	Next();
}

void Parser::HandleIdentifierToken_Emit()
{
	Logger::AddInfoMessage("Found Identifier Token (Label) on Second Pass\n");
	const std::string label = Utils::String::ToLower(Peek().value);

	if (m_labels.find(label) == m_labels.end())
	{
		AddError(std::format("Error at line {}, column {}: Undefined label '{}'.",
			Peek().line, Peek().column, Peek().value));
		ConsumeLine();
		return;
	}
	m_currentAddress = m_labels.at(label).address;
	m_currentStatement.address = m_currentAddress;

	ConsumeLabel();
}

void Parser::HandleOrgDirective()
{
	Next();
	ParsedNumber parsedNumber = ConsumeNumber();

	if (!CheckNumericLimit<uint16_t>(parsedNumber))
	{
		ConsumeLine();
		return;
	}

	if (parsedNumber.value < m_currentAddress)
	{
		AddWarning(std::format("Warning at line {}, column {}: '.org' address is lower than current one. Previous values could be overwritten.",
			Peek().line, Peek().column));
	}

	m_currentAddress = parsedNumber.value;
}

void Parser::HandleStartDirective()
{
	Next();
	
	if (m_seenStartDirective)
	{
		AddError(std::format("Error at line {}, column {}: '.start' directive already defined.",
			Peek().line, Peek().column));
		ConsumeLine();
		return;
	}

	if (Peek().type == TokenType::IDENTIFIER)
	{
		const std::string label = Utils::String::ToLower(Peek().value);
		if (m_labels.find(label) == m_labels.end())
		{
			AddError(std::format("Error at line {}, column {}: Undefined label '{}'.",
				Peek().line, Peek().column, Peek().value));
			ConsumeLine();
			return;
		}
		m_startAddress = m_labels.at(label).address;
	}
	else if (Peek().type == TokenType::NUMBER)
	{
		ParsedNumber parsedNumber = ConsumeNumber();
		if (!CheckNumericLimit<uint16_t>(parsedNumber))
		{
			ConsumeLine();
			return;
		}
		m_startAddress = parsedNumber.value;
	}
	else
	{
		AddError(std::format("Error at line {}, column {}: expected identifier or 16-bit address. Found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
		return;
	}
	m_seenStartDirective = true;
}

void Parser::HandleNewLineToken()
{
	ResetStatement();
	Next();
}

void Parser::HandleDotToken()
{
	Next();
	if (Peek().type != TokenType::IDENTIFIER)
	{
		AddError(std::format("Error at line {}, column {}: Expected identifier, found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
		return;
	}

	auto it = m_directiveHandlers.find(Utils::String::ToLower(Peek().value));
	if (it != m_directiveHandlers.end())
	{
		it->second();
	}
	else
	{
		AddError(std::format("Error at line {}, column {}: Unknown directive '{}'.",
			Peek().line, Peek().column, Peek().value));
		ConsumeLine();
		return;
	}

	Next();
}

void Parser::HandleDWToken_Scan()
{
	Next();
	if (Peek().type == TokenType::IDENTIFIER || Peek().type == TokenType::NUMBER)
	{
		m_currentAddress += 2;
		Next();
	}
	else
	{
		AddError(std::format("Error at line {}, column {}: Expected identifier or number, found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
	}
}

void Parser::HandleIdentifierToken_Scan()
{
	if (IsLabelDefinition())
	{
		const std::string label = Utils::String::ToLower(Peek().value);
		if (m_labels.find(label) != m_labels.end())
		{
			AddError(std::format("Error at line {}, column {}: Label '{}' already exists.",
				Peek().line, Peek().column, Peek().value));
			ConsumeLine();
		}
		else
		{
			Logger::AddInfoMessage(std::format("Label added: {}\n", label));
			m_labels.insert({ label, LabelInfo(m_currentAddress, Peek().line) });
		}
		ConsumeLabel();
	}
	else
	{
		ConsumeLine();
	}
}

std::array<uint8_t, 2> Parser::ConsumeImm8()
{
	std::array<uint8_t, 2> operand{};
	Next(); // go to imm8 value

	ParsedNumber parsedNumber = ConsumeNumber();
	if (!CheckNumericLimit<uint8_t>(parsedNumber)) return {};

	operand[0] = static_cast<uint8_t>(parsedNumber.value);
	Next(); // go to newline or end of file

	return operand;
}

std::array<uint8_t, 2> Parser::ConsumeAddr16()
{
	std::array<uint8_t, 2> operands{};
	uint16_t address{};
	const Token& token = Next(); // go to addr

	if (token.type == TokenType::IDENTIFIER)
	{
		const std::string label = Utils::String::ToLower(Peek().value);
		if (!m_labels.contains(label))
		{
			AddError(std::format("Error at line {}, {}: undefined label '{}'.",
				Peek().line, Peek().column, Peek().value));
			ConsumeLine();

			return {};
		}
		else
		{
			address = m_labels.at(label).address;
			m_labels.at(label).used = true;
			Next(); // go to newline or end of file
		}

	}
	else if (token.type == TokenType::NUMBER)
	{
		ParsedNumber parsedNumber = ConsumeNumber();
		if (!CheckNumericLimit<uint16_t>(parsedNumber)) return {};

		address = static_cast<uint16_t>(parsedNumber.value);
		Next();
	}
	else
	{
		AddError(std::format("Error at line {}, column {}: expected 16-bit address or label, found {}.",
			token.line, token.column, Utils::String::TokenTypeToString(token)));
		ConsumeLine();

		return {};
	}

	operands[0] = static_cast<uint8_t>(address >> 8); // hi part
	operands[1] = static_cast<uint8_t>(address & 0xFF); // lo part

	return operands;
}

std::array<uint8_t, 2> Parser::ConsumeReg()
{
	std::array<uint8_t, 2> operands{};
	operands[0] = ConsumeSelector();

	return operands;
}

std::array<uint8_t, 2> Parser::ConsumeRegReg()
{
	std::array<uint8_t, 2> operands{};

	operands[0] = ConsumeSelector();
	if (m_currentStatementHasError) return operands;

	ExpectComma();
	if (m_currentStatementHasError) return operands;

	operands[1] = ConsumeSelector();

	return operands;
}

void Parser::ExpectEndOfStatement()
{
	if (Peek().type != TokenType::NEW_LINE && Peek().type != TokenType::END_OF_FILE)
	{

		AddError(std::format("Error at line {}, column {}: expected newline or end of file at the end of statement. Found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
	}

	if (m_pos + 1 < m_tokens.size())
	{
		Next();
	}
}

void Parser::ExpectComma()
{
	if (Peek().type != TokenType::COMMA)
	{
		m_currentStatementHasError = true;

		AddError(std::format("Error at line {}, column {}: expected ',' after first operand of 'MOV', found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
	}
}

void Parser::ExpectColon()
{
	if (Next().type != TokenType::COLON)
	{
		m_currentStatementHasError = true;

		AddError(std::format("Error at line {}, column {}: expected ':' after label definition, found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();
	}
}

void Parser::CheckUnusedLabels()
{
	for (const auto& [label, info] : m_labels)
	{
		if (!info.used)
		{
			AddWarning(std::format("Warning at line {}: label '{}' is defined but never used.",
				info.lineDeclaration, label));
		}
	}
}

void Parser::ConsumeLabel()
{
	Next(); Next();
}

bool Parser::IsLabelDefinition() const
{
	return m_pos + 1 < m_tokens.size() && m_tokens[m_pos + 1].type == TokenType::COLON;
}

const Token& Parser::Peek() const
{
	return m_tokens[m_pos];
}

const Token& Parser::Next()
{
	if (m_pos == m_tokens.size() - 1) return m_tokens[m_pos];
	return m_tokens[++m_pos];
}

ParsedNumber Parser::ConsumeNumber()
{
	if (Peek().type != TokenType::NUMBER)
	{
		AddError(std::format("Error at line {}, column {}: expected number, found {}.",
			Peek().line, Peek().column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();

		return { 0u, false };
	}

	return Utils::String::ParseNumber(Peek().value);
}

uint8_t Parser::ConsumeSelector()
{
	const Token& token = Next();

	if (token.type == TokenType::REGISTER)
	{
		const std::string upper = Utils::String::ToUpper(token.value);
		if (!nameToSelector.contains(upper))
		{
			m_currentStatementHasError = true;

			AddError(std::format("Error at line {}, column {}: '{}' is not a valid register name (valid range: A-D).",
				token.line, token.column, token.value));
			ConsumeLine();
		}

		Next();
		return nameToSelector.at(upper);
	}
	else if (token.type == TokenType::NUMBER)
	{
		ParsedNumber parsedNumber = ConsumeNumber();
		if (!CheckNumericLimit<uint8_t>(parsedNumber)) return 0u;

		if (!selectorToName.contains(static_cast<uint8_t>(parsedNumber.value)))
		{
			m_currentStatementHasError = true;

			AddError(std::format("Error at line {}, column {}: '{}' is not a valid register selector (valid range: 0x00-0x03).",
				token.line, token.column, parsedNumber.value));
			ConsumeLine();

			return 0u;
		}

		Next();
		return static_cast<uint8_t>(parsedNumber.value);
	}
	else
	{
		m_currentStatementHasError = true;

		AddError(std::format("Error at line {}, column {}: expected register selector or name, found {}.",
			token.line, token.column, Utils::String::TokenTypeToString(Peek())));
		ConsumeLine();

		return 0u;
	}
}

void Parser::ConsumeLine()
{
	Logger::AddInfoMessage("CONSUME LINE called\n\n");
	while (Peek().type != TokenType::NEW_LINE && Peek().type != TokenType::END_OF_FILE)
	{
		Next();
	}
	Logger::AddInfoMessage("CONSUME LINE finished\n");
}
