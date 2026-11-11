#pragma once

#include "Debugger/Operand.h"
#include "Debugger/Group.h"

#include <string>
#include <unordered_map>

struct InstructionDef
{
	std::string_view mnemonic{};
	Operand op1 = Operand::None;
	Operand op2 = Operand::None;
	Group group = Group::None;
};

const inline std::unordered_map<uint16_t, std::string> selectorToName{
	{0x00, "A"},
	{0x01, "B"},
	{0x02, "C"},
	{0x03, "D"} };

const inline std::unordered_map<std::string, uint16_t> nameToSelector{
	{"A", 0x00},
	{"B", 0x01},
	{"C", 0x02},
	{"D", 0x03} };