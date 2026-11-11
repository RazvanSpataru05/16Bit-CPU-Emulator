#pragma once

#include "Debugger/InstructionDef.h"
#include "Debugger/DecodedInstruction.h"

#include <array>
#include <string>
#include <format>
#include <functional>

class Dissasembler
{
public:
	using ReadFn = std::function<uint8_t(uint16_t)>;

	explicit Dissasembler();

	[[nodiscard]] DecodedInstruction Decode(uint16_t offset, const ReadFn& read) const;
	[[nodiscard]] const InstructionDef& GetInstructionDef(uint8_t index) const noexcept;

private:
	void Set(int opcode, std::string_view mnemonic, Operand op1 = Operand::None,
		Operand op2 = Operand::None, Group group = Group::None);

private:
	std::array<InstructionDef, 256> m_table{};
};