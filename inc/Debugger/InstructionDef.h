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

const inline std::unordered_map<uint8_t, std::string> selectorToReg8{
	{0x00, "AL"},
	{0x01, "CL"},
	{0x02, "DL"},
	{0x03, "BL"},
	{0x04, "AH"},
	{0x05, "CH"},
	{0x06, "DH"},
	{0x07, "BH"}
};

const inline std::unordered_map<std::string, uint8_t> reg8ToSelector{
	{"AL", 0x00},
	{"CL", 0x01},
	{"DL", 0x02},
	{"BL", 0x03},
	{"AH", 0x04},
	{"CH", 0x05},
	{"DH", 0x06},
	{"BH", 0x07}
};

const inline std::unordered_map<uint8_t, std::string> selectorToReg16{
	{0x00, "AX"},
	{0x01, "CX"},
	{0x02, "DX"},
	{0x03, "BX"},
	{0x04, "SP"},
	{0x05, "BP"},
	{0x06, "SI"},
	{0x07, "DI"}
};

const inline std::unordered_map<std::string, uint8_t> reg16ToSelector{
	{"AX", 0x00},
	{"CX", 0x01},
	{"DX", 0x02},
	{"BX", 0x03},
	{"SP", 0x04},
	{"BP", 0x05},
	{"SI", 0x06},
	{"DI", 0x07}
};

const inline std::unordered_map<uint8_t, std::string> selectorToSegmentReg{
	{0x00, "ES"},
	{0x01, "CS"},
	{0x02, "SS"},
	{0x03, "DS"},
};

const inline std::unordered_map<std::string, uint8_t> segmentRegToSelector{
	{"ES", 0x00},
	{"CS", 0x01},
	{"SS", 0x02},
	{"DS", 0x03},
};