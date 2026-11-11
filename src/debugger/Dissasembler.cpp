#include "Debugger/Dissasembler.h"

namespace
{
	constexpr std::array<std::string_view, 8> kReg8 = { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" };
	constexpr std::array<std::string_view, 8> kReg16 = { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" };
	constexpr std::array<std::string_view, 4> kSeg = { "ES", "CS", "SS", "DS" };
	constexpr std::array<std::string_view, 8> kBase = { "BX+SI", "BX+DI", "BP+SI", "BP+DI", "SI", "DI", "BP", "BX" };

	constexpr std::array<std::array<std::string_view, 8>, 6> kGroupNames = { {
		{},																	// None
		{"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"},			// Grp1
		{"ROL", "ROR", "RCL", "RCR", "SHL", "SHR", "", "SAR"},				// Grp2
		{"TEST", "", "NOT", "NET", "MUL", "IMUL", "DIV", "IDIV"},			// Grp3a/b
		{"INC", "DEC", "", "", "", "", "", ""},								// Grp4
		{"INC", "DEC", "CALL", "CALL FAR", "JMP", "JMP FAR", "PUSH", ""}	// Grp5
	} };

	bool UsesModRM(Operand o)
	{
		switch (o)
		{
		case Operand::Eb:
		case Operand::Ev:
		case Operand::Ew:
		case Operand::Gb:
		case Operand::Gv:
		case Operand::Sw:
		case Operand::M:
			return true;
		default:
			return false;
		}
	}

	bool GivesSizeHint(Operand o)
	{
		switch (o)
		{
		case Operand::Gb:
		case Operand::Gv:
		case Operand::Sw:
		case Operand::AL:
		case Operand::AX:
			return true;
		default:
			return false;
		}
	}

}

Dissasembler::Dissasembler()
{
	using enum Operand;

	constexpr std::array<std::string_view, 8> alu =
	{
		"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"
	};

	for (int op = 0; op < 8; ++op)
	{
		const int base = op * 8;
		Set(base + 0, alu[op], Eb, Gb);
		Set(base + 1, alu[op], Ev, Gv);
		Set(base + 2, alu[op], Gb, Eb);
		Set(base + 3, alu[op], Gv, Ev);
		Set(base + 4, alu[op], AL, Ib);
		Set(base + 5, alu[op], AX, Iv);
	}

	for (int s = 0; s < 4; ++s)
	{
		Set(0x06 + s * 8, "PUSH", SegOp);
		Set(0x07 + s * 8, "POP", SegOp);
	}

	for (int r = 0; r < 8; ++r)
	{
		Set(0x40 + r, "INC", Reg160p);
		Set(0x48 + r, "DEC", Reg160p);
		Set(0x50 + r, "PUSH", Reg160p);
		Set(0x58 + r, "POP", Reg160p);
		Set(0xB0 + r, "MOV", Reg80p, Ib);
		Set(0xBB + r, "MOV", Reg160p, Iv);
	}

	constexpr std::array<std::string_view, 16> jcc = {
		"JO", "JNO", "JB", "JNB",
		"JZ", "JNZ", "JBE", "JA",
		"JS", "JNS", "JPE", "JPO"
		"JL", "JGE", "JLE", "JG"
	};
	for (int i = 0; i < 16; ++i)
	{
		Set(0x70 + i, jcc[i], Jb);
	}

	Set(0x80, {}, Eb, Ib, Group::Grp1);
	Set(0x81, {}, Ev, Iv, Group::Grp1);
	Set(0x82, {}, Eb, Ib, Group::Grp1);
	Set(0x83, {}, Ev, IbSx, Group::Grp1);

	Set(0x84, "TEST", Eb, Gb);
	Set(0x85, "TEST", Ev, Gv);
	Set(0x86, "XCHG", Eb, Gb);
	Set(0x87, "XCHG", Ev, Gv);
	Set(0x88, "MOV", Eb, Gb);
	Set(0x89, "MOV", Ev, Gv);
	Set(0x8A, "MOV", Gb, Eb);
	Set(0x8B, "MOV", Gv, Ev);
	Set(0x8C, "MOV", Ew, Sw);
	Set(0x8D, "LEA", Gv, M);
	Set(0x8E, "MOV", Sw, Ew);
	Set(0x8F, "POP", Ev);

	Set(0x90, "NOP");

	for (int r = 1; r < 8; ++r)
	{
		Set(0x90 + r, "XCHG", Reg160p, AX);
	}

	Set(0x98, "CBW");
	Set(0x99, "CWD");
	Set(0x9A, "CALL", Ap);

	Set(0xA0, "MOV", AL, Ob);
	Set(0xA1, "MOV", AX, Ov);
	Set(0xA2, "MOV", Ob, AL);
	Set(0xA3, "MOV", Ov, AX);

	Set(0xC2, "RET", Iv);
	Set(0xC3, "RET");
	Set(0xCA, "RETF", Iv);
	Set(0xCB, "RETF");
	Set(0xCC, "INT", Three);
	Set(0xCD, "INT", Ib);
	Set(0xCF, "IRET");

	Set(0xD0, {}, Eb, One, Group::Grp2);
	Set(0xD1, {}, Ev, One, Group::Grp2);
	Set(0xD2, {}, Eb, CL, Group::Grp2);
	Set(0xD3, {}, Ev, CL, Group::Grp2);

	Set(0xE8, "CALL", Jv);
	Set(0xE9, "JMP", Jv);
	Set(0xEA, "JMP", Ap);
	Set(0xEB, "JMP", Jb);

	Set(0xF4, "HLT");
	Set(0xFE, {}, Eb, None, Group::Grp4);
	Set(0xFE, {}, Ev, None, Group::Grp5);
}

void Dissasembler::Set(int opcode, std::string_view mnemonic, Operand op1, Operand op2, Group group)
{
	m_table[static_cast<size_t>(opcode)] = { mnemonic, op1, op2, group };
}

DecodedInstruction Dissasembler::Decode(uint16_t offset, const ReadFn& read) const
{
	uint16_t pos = offset;
	const auto fetch8 = [&]() -> uint8_t { return read(pos++); };
	const auto fetch16 = [&]() -> uint16_t
		{
			const uint8_t lo = read(pos++);
			const uint8_t hi = read(pos++);
			return static_cast<uint16_t>((hi << 8) | lo);
		};

	std::string prefix;
	std::string_view segOverride{};

	for (int i = 0; i < 4; ++i)
	{
		const uint8_t b = read(pos);
		if (b == 0x26 || b == 0x2E || b == 0x36 || b == 0x3E) segOverride = kSeg[(b >> 3) & 3];
		else if (b == 0xF0) prefix += "LOCK ";
		else if (b == 0xF2) prefix += "REPNZ ";
		else if (b == 0xF3) prefix += "REPZ ";
		else break;
		++pos;
	}

	const std::string segText = segOverride.empty() ? std::string{} : std::string(segOverride) + ":";

	const uint8_t opcode = fetch8();
	const InstructionDef& def = m_table[opcode];
	if (def.mnemonic.empty() && def.group == Group::None)
	{
		return { std::format("DB 0x{:02X}", read(offset)), 1 };
	}

	uint8_t mod = 3, reg = 0, rm = 0;
	std::string ea; // effective addr

	if (UsesModRM(def.op1) || UsesModRM(def.op2))
	{
		const uint8_t modrm = fetch8();
		mod = modrm >> 6;
		reg = (modrm >> 3) & 7;
		rm = modrm & 7;

		if (mod != 3)
		{
			std::string inner;
			if (mod == 0 && rm == 6)
			{
				inner = std::format("0x{:04x}", fetch16());
			}
			else
			{
				inner = std::string(kBase[rm]);
				if (mod == 1)
				{
					const int disp = static_cast<int8_t>(fetch8());
					inner += std::format("{}0x{:02X}", disp < 0 ? '-' : '+', disp < 0 ? -disp : disp);
				}
				else if (mod == 2)
				{
					const int disp = static_cast<int16_t>(fetch16());
					inner += std::format("{}0x{:04X}", disp < 0 ? '-' : '+', disp < 0 ? -disp : disp);
				}
			}
			ea = std::format("{}[{}]", segText, inner);
		}
	}

	std::string_view mnemonic = def.mnemonic;
	if (def.group != Group::None)
	{
		mnemonic = kGroupNames[static_cast<size_t>(def.group)][reg];
		if (mnemonic.empty()) return { std::format("DB 0x{:02x}", read(offset)), 1 };
	}

	const auto formatOperand = [&](Operand o, Operand other) -> std::string
		{
			switch (o)
			{
			case Operand::Eb:
			case Operand::Ev:
			case Operand::Ew:
			{
				const bool word = (o != Operand::Eb);
				if (mod == 3) return std::string(word ? kReg16[rm] : kReg8[rm]);
				const std::string size = GivesSizeHint(other) ? "" : (word ? "WORD PTR" : "BYTE PTR");
				return size + ea;
			}
			case Operand::M:	return mod == 3 ? std::string("???") : ea;
			case Operand::Gb:	return std::string(kReg8[reg]);
			case Operand::Gv:	return std::string(kReg16[reg]);
			case Operand::Sw:	return std::string(kSeg[reg & 3]);
			case Operand::Ib:	return std::format("0x{:02X}", fetch8());
			case Operand::IbSx: return std::format("0x{:04X}", static_cast<uint16_t>(static_cast<int8_t>(fetch8())));
			case Operand::Iv:	return std::format("0x{:04X}", fetch16());
			case Operand::Jb:
			{
				const int8_t relative = static_cast<int8_t>(fetch8());
				return std::format("0x{:04X}", static_cast<uint16_t>(pos + relative));
			}
			case Operand::Jv:
			{
				const uint16_t relative = fetch16();
				return std::format("0x:{:04x}", static_cast<uint16_t>(pos + relative));
			}
			case Operand::Ap:
			{
				const uint16_t offset = fetch16();
				const uint16_t segment = fetch16();
				return std::format("0x{:04X}:0x{:04X}", segment, offset);
			}
			case Operand::Ob:
			case Operand::Ov:
			{
				const uint16_t addr = fetch16();
				return std::format("{}[0x{:04X}", segText, addr);
			}
			case Operand::Reg80p:	return std::string(kReg8[opcode & 7]);
			case Operand::Reg160p:	return std::string(kReg16[opcode & 7]);
			case Operand::SegOp:	return std::string(kSeg[(opcode >> 3) & 3]);
			case Operand::AL:		return "AL";
			case Operand::AX:		return "AX";
			case Operand::CL:		return "CL";
			case Operand::DX:		return "DX";
			case Operand::One:		return "1";
			case Operand::Three:	return "3";
			default:				return {};
			}
		};

	const std::string first = formatOperand(def.op1, def.op2);
	const std::string second = formatOperand(def.op2, def.op1);

	std::string text = prefix + std::string(mnemonic);
	if (!first.empty()) text += " " + first;
	if (!second.empty()) text += ", " + second;

	return { std::move(text), static_cast<uint16_t>(pos - offset) };
}

const InstructionDef& Dissasembler::GetInstructionDef(uint8_t index) const noexcept
{
	return m_table[index];
}
