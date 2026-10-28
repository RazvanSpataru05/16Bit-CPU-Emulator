#include "Core/CPU.h"

CPU::CPU(InterruptController& interruptController) :
	m_interruptController{ interruptController }
{
	Reset();
}

uint20_t CPU::PhysAddr(uint16_t segment, uint16_t offset) const noexcept
{
	return uint20_t(static_cast<uint32_t>((segment << 4) + offset));
}

uint8_t CPU::Fetch8()
{
	++m_IP;
	assert(m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].has_value());
	return m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value();
}

uint16_t CPU::Fetch16()
{
	++m_IP;
	assert(m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].has_value());
	const uint8_t lo = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value();

	++m_IP;
	assert(m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].has_value());
	const uint8_t hi = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value();

	return static_cast<uint16_t>((hi << 8) | lo);
}

void CPU::Step()
{
	if (m_haltFlag || m_memoryUnit.IsMemoryEmpty() ||
		!m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].has_value()) return;

	if ((m_IR & 0xC0) == 0x00 && (m_IR & 0b111) < 6) ExecALU();
	/*
	^	opcodes:	00 -> 05 (ADD);
					08 -> 0D (OR);
					10 -> 15 (ADC);
					18 -> 1D (SBB);
					20 -> 25 (AND);
					28 -> 2D (SUB);
					30 -> 35 (XOR);
					38 -> 3D (CMP);
		total: 48 instructions
	*/
	else if ((m_IR & 0xE6) == 0x06)					 ExecPushPopSeg(); // opcodes: {06, 07, 0E, 0F, 16, 17, 1E, 1F} (8 instr)
	else if ((m_IR & 0xF0) == 0x40)					 ExecINCDEC(); // opcodes: 40 -> 4F (16 instr)
	else if ((m_IR & 0xF0) == 0x50)					 ExecPushPopGP(); // opcodes: 50 -> 5F (16 instr) 
	// opcodes 60-6f are undefined 
	else if ((m_IR & 0xF0) == 0x70)					 ExecConditionedJump(); // opcodes: 70 -> 7F (16 instr)
	else if ((m_IR & 0xFC) == 0x88)					 ExecMOVModRM();		// opcodes: 88 -> 8B (4 instr)
	else if ((m_IR & 0xF8) == 0x90)					 ExecExchange();		// opcodes: 90 -> 97 (8 instr)
	else if ((m_IR & 0xFE) == 0x98)					 ExecConvert();			// opcodes: 98, 99 (CBW, CWD)
	else if ((m_IR & 0xFE) == 0x9C)					 ExecPushPopFlags();	// opcodes: 9C, 9D (PUSHF, POPF) 
	else if ((m_IR & 0xFE) == 0x9E)					 ExecStoreRegIntoFlags(); // opcodes: 9E, 9F (SAHF, LAHF)
	else if ((m_IR & 0xF0) == 0xB0)					 ExecMOVImm();			// opcodes: B0 -> BF (16 instr)
	else if ((m_IR & 0xFE) == 0xC2)					 ExecRet();				// opcodes: C2, C3 (RET Iw, RET)
	else if ((m_IR & 0xFE) == 0xCA)					 ExecRetFar();			// opcodes: CA, CB (RETF Iw, RETF)
	else if ((m_IR & 0xFC) == 0xE0)					 ExecLoop();			// opcodes: E0 -> E4 (LOOPNZ / LOOPNE, LOOPE / LOOPZ, LOOP, JCXZ)
	else if ((m_IR & 0xFC) == 0xF8)					 ExecClearStoreFlags(); // opcodes: F8 -> FD (CLC, STC, CLI, STI, CLD, STD)
	else switch (m_IR)
	{
	case 0x37:			// AAA (ASCII Adjust after Addition)
	{
		const uint8_t lowNibble = m_registerFile.AL() & Utils::Constants::LOW_NIBBLE_MASK;
		if (lowNibble > 9 || m_statusRegister.Auxiliary())
		{
			m_registerFile.Set8(Reg8::AL, m_registerFile.AL() + 6);
			m_registerFile.Set8(Reg8::AH, m_registerFile.AH() + 1);
			m_statusRegister.SetFlagOn(Utils::Constants::AF_MASK);
			m_statusRegister.SetFlagOn(Utils::Constants::CF_MASK);
		}
		else
		{
			m_statusRegister.SetFlagOff(Utils::Constants::AF_MASK);
			m_statusRegister.SetFlagOff(Utils::Constants::CF_MASK);
		}
		m_registerFile.Set8(Reg8::AL, m_registerFile.AL() & Utils::Constants::LOW_NIBBLE_MASK);
		++m_IP;

		break;
	}
	case 0x3F:		// AAS (ASCII Adjust after Subtraction)
	{
		const uint8_t lowNibble = m_registerFile.AL() & Utils::Constants::LOW_NIBBLE_MASK;
		if (lowNibble > 9 || m_statusRegister.Auxiliary())
		{
			m_registerFile.Set8(Reg8::AL, m_registerFile.AL() - 6);
			m_registerFile.Set8(Reg8::AH, m_registerFile.AH() - 1);
			m_statusRegister.SetFlagOn(Utils::Constants::AF_MASK);
			m_statusRegister.SetFlagOn(Utils::Constants::CF_MASK);
		}
		else
		{
			m_statusRegister.SetFlagOff(Utils::Constants::AF_MASK);
			m_statusRegister.SetFlagOff(Utils::Constants::CF_MASK);
		}
		m_registerFile.Set8(Reg8::AL, m_registerFile.AL() & Utils::Constants::LOW_NIBBLE_MASK);
		++m_IP;
		break;
	}
	case 0x9A:			// CALL Ap (Address pointer)
	{
		const uint16_t segment = Fetch16();
		const uint16_t offset = Fetch16();
		++m_IP;

		Push16(m_registerFile.CS());
		Push16(m_IP);

		m_IP = offset;
		m_registerFile.CS() = segment;

		break;
	}
	case 0x9B:			// WAIT
	{
		if (!m_TESTPin) ++m_IP;

		break;
	}
	case 0xE8:			// CALL Jv (Near Call)
	{
		const int16_t disp = static_cast<int16_t>(Fetch16());
		++m_IP;
		Push16(m_IP);
		m_IP += disp;

		break;
	}
	case 0xE9:			// JMP Jv (Near jump)
	{
		const int16_t offset = static_cast<int16_t>(Fetch16());
		++m_IP;
		m_IP = static_cast<uint16_t>(m_IP + offset);
		break;
	}
	case 0xEA:			// JMP Ap (Far jump)
	{
		const uint16_t offset = Fetch16();
		const uint16_t segment = Fetch16();
		m_registerFile.Set16(SegmentReg::CS, segment);
		m_IP = offset;
		break;
	}
	case 0xEB:			// JMP Jb (Short jump)
	{
		const int8_t offset = static_cast<int8_t>(Fetch8());
		++m_IP;
		m_IP = static_cast<uint16_t>(m_IP + offset);
		break;
	}

	case 0xF4: // HLT 1by
	{
		m_haltFlag = true;
		break;
	}
	case 0xF5: // CMC 1by (Complement Carry Flag)
	{
		m_statusRegister.Carry() ? m_statusRegister.SetFlagOff(Utils::Constants::CF_MASK)
			: m_statusRegister.SetFlagOn(Utils::Constants::CF_MASK);
		++m_IP;
		break;
	}
	case 0xFE: // GRP4 Eb (INC, DEC)
	case 0xFF: // GRP5 Ev (INC, DEC, CALL Ev, CALL Mp, JMP Ev, JMP Mp, PUSH Ev)
	{
		const bool isWord = (m_IR & 1) == 1;
		ModRM modRM = DecodeModRM();
		++m_IP;
		if (isWord)
		{

		}
		else
		/*
		* ModRM structure: ab cde fgh
		* ModRM notation:
		* ab -> mod (only set to 11 for this instruction)
		* cde -> reg (000 -> Increment 8-bit reg | 001 -> Decrement 8-bit reg)
		* fgh -> rm (selector for 8-bit reg (AL = 000 ...... BH = 111))
		*/
		{
			const uint8_t reg = ReadReg(modRM.rm, isWord);
			Result r;

			if (modRM.reg == 0)
			{
				r = ALU::INC(reg, isWord);
			}
			else
			{
				r = ALU::DEC(reg, isWord);
			}

			WriteReg(modRM.rm, r.value, isWord);
			m_statusRegister.UpdateFrom(r);
		}

		break;
	}

	default:
		Logger::AddInfoMessage(std::format("Unimplemented opcode: {:02X}", m_IR));
		++m_IP;
		break;
	}
	UpdateIR();
}

void CPU::ExecALU()
{
	const uint8_t operation = (m_IR >> 3) & 0b111;
	const uint8_t form = m_IR & 0b111;
	if (form < 4)
	{
		ModRM modRM = DecodeModRM();
		if (modRM.mod != 0b11) // NOT reg-reg
		{
			Logger::AddInfoMessage(std::format("Unimplemented mod: {}",
				static_cast<uint8_t>(modRM.mod)));
		}

		const bool word = (form & 0b1) != 0;	// bit 1 of opcode, word = 16-bits, else 8-bits
		const bool toReg = (form & 0b10) != 0;	// if bit 1 is true, dest is reg

		const uint8_t destination = toReg ? modRM.reg : modRM.rm;
		const uint8_t source = toReg ? modRM.rm : modRM.reg;

		const Result r = ALU::Execute(operation, ReadReg(destination, word), ReadReg(source, word),
			word, m_statusRegister.Carry());

		m_statusRegister.UpdateFrom(r);
		if (operation != 0b111) // not CMP
		{
			WriteReg(destination, r.value, word);
		}
	}
	else
	{
		const bool word = (form == 5);		// 4 = AL, imm8		5 = AX, imm16
		const uint16_t a = word ? m_registerFile.Get16(Reg16::AX) : m_registerFile.Get8(Reg8::AL);
		const uint16_t b = word ? Fetch16() : Fetch8();

		const Result r = ALU::Execute(operation, a, b, word, m_statusRegister.Carry());
		m_statusRegister.UpdateFrom(r);

		if (operation != 0b111) // not CMP
		{
			if (word) m_registerFile.Set16(Reg16::AX, static_cast<uint16_t>(r.value));
			else m_registerFile.Set8(Reg8::AL, static_cast<uint8_t>(r.value));
		}
	}
	++m_IP;
}

void CPU::ExecPushPopGP()
{
	const uint8_t selector = m_IR & 0b111;
	const bool isPush = ((m_IR >> 3) & 1) == 0;
	StackOperation(static_cast<Reg16>(selector), isPush);
}

void CPU::ExecPushPopSeg()
{
	const uint8_t selector = (m_IR >> 3) & 0b11;
	const bool isPush = (m_IR & 1) == 0;
	StackOperation(static_cast<SegmentReg>(selector), isPush);
}

void CPU::ExecPushPopFlags()
{
	const bool isPush = (m_IR & 1) == 0;
	if (isPush)
	{
		const uint16_t sp = static_cast<uint16_t>(m_registerFile.Get16(Reg16::SP) - 2);
		const uint16_t flags = m_statusRegister.GetFlags();
		m_registerFile.Set16(Reg16::SP, sp);

		const uint8_t lo = static_cast<uint8_t>(flags & 0xFF);
		const uint8_t hi = static_cast<uint8_t>(flags >> 8);

		m_memoryUnit[PhysAddr(m_registerFile.SS(), sp)] = lo;
		m_memoryUnit[PhysAddr(m_registerFile.SS(), static_cast<uint16_t>(sp + 1))] = hi;
	}
	else
	{
		const uint16_t sp = static_cast<uint16_t>(m_registerFile.SP());
		const uint16_t word = ReadStackWord(sp);
		m_registerFile.Set16(Reg16::SP, sp + 2);
		m_statusRegister.SetFlags(word);
	}
	++m_IP;
}

void CPU::ExecMOVImm()
{
	const bool word = (m_IR >> 3) & 1; // word == 1 -> full reg / word == 0 -> Hi / Lo
	const uint8_t selector = m_IR & 0b111;

	if (word)
	{
		m_registerFile.Set16(static_cast<Reg16>(selector), Fetch16());
	}
	else
	{
		m_registerFile.Set8(static_cast<Reg8>(selector), Fetch8());
	}
	++m_IP;
}

void CPU::ExecRet()
{
	const bool isRetIw = (m_IR & 1) == 0;
	const uint16_t returnAddr = Pop16();

	if (isRetIw)
	{
		const uint16_t value = Fetch16();
		m_registerFile.SP() += value;
	}
	m_IP = returnAddr;
}

void CPU::ExecRetFar()
{
	const bool isRetIw = (m_IR & 1) == 0;
	const uint16_t offset = Pop16();
	const uint16_t segment = Pop16();

	if (isRetIw)
	{
		const uint16_t value = Fetch16();
		m_registerFile.SP() += value;
	}
	m_IP = offset;
	m_registerFile.CS() = segment;
}

void CPU::ExecMOVModRM()
{
	ModRM modRM = DecodeModRM();
	if (modRM.mod != 0b11)
	{
		Logger::AddInfoMessage(std::format("Unimplemented mod: {}", static_cast<uint8_t>(modRM.mod)));
		return;
	}

	const bool word = (m_IR & 0b1) != 0;
	const bool toReg = (m_IR & 0b10) != 0;

	const uint8_t destination = toReg ? modRM.reg : modRM.rm;
	const uint8_t source = toReg ? modRM.rm : modRM.reg;

	WriteReg(destination, ReadReg(source, word), word);
	++m_IP;
}

void CPU::ExecINCDEC()
{
	const uint8_t selector = m_IR & 0b111;
	const bool isDEC = ((m_IR >> 3) & 0b1) != 0;
	Result r;

	if (isDEC)
	{
		r = ALU::DEC(m_registerFile.Get16(static_cast<Reg16>(selector)), true);
	}
	else
	{
		r = ALU::INC(m_registerFile.Get16(static_cast<Reg16>(selector)), true);
	}

	m_statusRegister.UpdateFrom(r);
	m_registerFile.Set16(static_cast<Reg16>(selector), r.value);
	++m_IP;
}

void CPU::ExecLoop()
{
	const uint8_t loopType = (m_IR & 0b11);
	const int8_t disp = static_cast<int8_t>(Fetch8());
	bool condition{ false };
	++m_IP;

	switch (loopType)
	{
	case 0b00: // LOOPNZ / LOOPNE (E0)
	{
		--m_registerFile.CX();
		condition = m_registerFile.CX() != 0 && m_statusRegister.Zero() == 0;
		break;
	}
	case 0b01: // LOOPZ / LOOPE (E1)
	{
		--m_registerFile.CX();
		condition = m_registerFile.CX() != 0 && m_statusRegister.Zero() == 1;
		break;
	}
	case 0b10: // LOOP (E2)
	{
		--m_registerFile.CX();
		condition = m_registerFile.CX() != 0;
		break;
	}
	case 0b11: // JCXZ (E3)
	{
		condition = m_registerFile.CX() == 0;
		break;
	}
	}

	if (condition)
	{
		m_IP += disp;
	}
}

void CPU::ExecClearStoreFlags()
{
	const bool isClear = (m_IR & 1) == 0;
	const uint8_t flag = (m_IR >> 1) & 0b11;

	switch (flag)
	{
	case 0b00: // Carry Flag
	{
		isClear ? m_statusRegister.SetFlagOff(Utils::Constants::CF_MASK)
			: m_statusRegister.SetFlagOn(Utils::Constants::CF_MASK);
		break;
	}
	case 0b01: // Interrupt Flag
	{
		isClear ? m_statusRegister.SetFlagOff(Utils::Constants::IF_MASK)
			: m_statusRegister.SetFlagOn(Utils::Constants::IF_MASK);
		break;
	}
	case 0b10: // Directional Flag
	{
		isClear ? m_statusRegister.SetFlagOff(Utils::Constants::DF_MASK)
			: m_statusRegister.SetFlagOn(Utils::Constants::DF_MASK);
		break;
	}
	default:
	{
		std::cout << "Unknown opcode: " << std::hex << static_cast<int>(m_IR) << '\n';
		break;
	}
	}
	++m_IP;
}

void CPU::ExecConditionedJump()
{
	const bool negative = (m_IR & 0b1) == 1;
	const uint8_t flag = (m_IR >> 1) & 0b111;
	const int8_t offset = static_cast<int8_t>(Fetch8());
	bool result = false;
	++m_IP;

	switch (flag)
	{
	case 0b000:		// JO & JNO
	{
		result = m_statusRegister.Overflow() ^ negative;
		break;
	}
	case 0b001:		// JB & JNB
	{
		result = m_statusRegister.Carry() ^ negative;
		break;
	}
	case 0b010:		// JZ & JNZ
	{
		result = m_statusRegister.Zero() ^ negative;
		break;
	}
	case 0b011:		// JBE (CF = 1 or ZF = 1) & JA (CF = 0 and ZF = 0)
	{
		result = (m_statusRegister.Carry() || m_statusRegister.Zero()) ^ negative;
		break;
	}
	case 0b100:		// JS & JNS
	{
		result = m_statusRegister.Sign() ^ negative;
		break;
	}
	case 0b101:		// JPE & JPO
	{
		result = m_statusRegister.Parity() ^ negative;
		break;
	}
	case 0b110:		// JL (SF != OF) & JGE (SF == OF)
	{
		result = (m_statusRegister.Sign() != m_statusRegister.Overflow()) ^ negative;
		break;
	}
	case 0b111:		// JLE (SF != OF or ZF == 1) & JG (ZF == 0 and SF == OF)
	{
		result = ((m_statusRegister.Sign() != m_statusRegister.Overflow()) ||
			m_statusRegister.Zero()) ^ negative;
		break;
	}
	}

	if (result)
	{
		ShortJump(offset);
	}
}

void CPU::ExecExchange()
{
	const uint8_t selector = m_IR & 0b111;
	std::swap(m_registerFile.AX(),
		m_registerFile.Get16(static_cast<Reg16>(selector)));
	++m_IP;
}

void CPU::ExecStoreRegIntoFlags()
{
	const bool isStore = (m_IR & 1) == 0;
	if (isStore)
	{
		const uint8_t value = (m_registerFile.AH() & Utils::Constants::AH_MASK) | (1u << 1);
		m_statusRegister.SetLoFlags(value);
	}
	else
	{
		const uint8_t loFlags = m_statusRegister.GetLoFlags();
		m_registerFile.Set8(Reg8::AL, loFlags);
	}
	++m_IP;
}

void CPU::ExecConvert()
{
	const bool isConvertByte = (m_IR & 1) == 0;
	if (isConvertByte)
	{
		const int8_t al = static_cast<int8_t>(m_registerFile.AL());
		if (al < 0)
		{
			m_registerFile.Set8(Reg8::AH, std::numeric_limits<uint8_t>::max());
		}
		else
		{
			m_registerFile.Set8(Reg8::AH, 0u);
		}
	}
	else
	{
		const int16_t ax = static_cast<int16_t>(m_registerFile.AX());
		if (ax < 0)
		{
			m_registerFile.Set16(Reg16::DX, std::numeric_limits<uint16_t>::max());
		}
		else
		{
			m_registerFile.Set16(Reg16::DX, 0u);
		}
	}
	++m_IP;
}

void CPU::Reset()
{
	m_registerFile.Reset();
	m_statusRegister.Reset();
	m_haltFlag = false;
	m_TESTPin = false;

	m_IP = m_memoryUnit.GetStartAddress();

	if (m_memoryUnit[m_IP].has_value())
	{
		m_IR = m_memoryUnit[m_IP].value();
	}
	else
	{
		m_IR = 0x00;
	}

}

ModRM CPU::DecodeModRM()
{
	ModRM modRM;
	modRM.raw = Fetch8();

	modRM.rm = modRM.raw & 0b111;
	modRM.reg = (modRM.raw >> 3) & 0b111;
	modRM.mod = (modRM.raw >> 6) & 0b11;
	return modRM;
}

void CPU::HandleInterrupt()
{
	if (const auto result = m_interruptController.PollPendingInterrupt(); result.has_value())
	{
		const uint16_t line = result.value();

		const uint16_t hiPC = (m_IP >> 8) & 0xFF;
		const uint16_t loPC = m_IP & 0xFF;

		// save PC address on the stack
		m_memoryUnit[m_registerFile.SP()--] = hiPC;
		m_memoryUnit[m_registerFile.SP()--] = loPC;

		const uint16_t vectorAddress = line * 2;
		m_IP = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value();
	}
}

uint16_t CPU::GetIP() const noexcept
{
	return m_IP;
}

uint8_t CPU::GetIR() const noexcept
{
	return m_IR;
}

bool CPU::GetHaltFlag() const noexcept
{
	return m_haltFlag;
}

void CPU::SetInstructionPointer(uint16_t address)
{
	m_IP = address;
}

void CPU::SetInstructionRegister()
{
	if (!m_memoryUnit[m_IP].has_value())
	{
		m_haltFlag = true;
		return;
	}
	m_IR = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value();
}

void CPU::UpdateIR()
{
	if (m_haltFlag) return;

	//assert(m_memoryUnit[PhysAddr()].has_value());
	if (m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].has_value())
	{
		m_IR = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value();
	}
	else
	{
		m_IR = 0x00;
	}

}

bool CPU::IsReadingInstruction() const noexcept
{
	return m_memoryUnit.Read(m_IP) == 0x02;
}

bool CPU::IsWritingInstruction() const noexcept
{
	return m_memoryUnit.Read(m_IP) == 0x04;
}

const MemoryUnit& CPU::GetMemoryUnit() const noexcept
{
	return m_memoryUnit;
}

MemoryUnit& CPU::GetMemoryUnit() noexcept
{
	return m_memoryUnit;
}

const InterruptController& CPU::GetInterruptController() const noexcept
{
	return m_interruptController;
}

InterruptController& CPU::GetInterruptController() noexcept
{
	return m_interruptController;
}

const RegisterFile& CPU::GetRegisterFile() const noexcept
{
	return m_registerFile;
}

RegisterFile& CPU::GetRegisterFile() noexcept
{
	return m_registerFile;
}

const StatusRegister& CPU::GetStatusRegister() const noexcept
{
	return m_statusRegister;
}

StatusRegister& CPU::GetStatusRegister() noexcept
{
	return m_statusRegister;
}

uint16_t CPU::ReadReg(uint8_t selector, bool word) const
{
	if (word) return m_registerFile.Get16(static_cast<Reg16>(selector));
	return m_registerFile.Get8(static_cast<Reg8>(selector));
}

void CPU::WriteReg(uint8_t selector, uint16_t value, bool word)
{
	if (word)
	{
		m_registerFile.Set16(static_cast<Reg16>(selector), value);
		return;
	}
	m_registerFile.Set8(static_cast<Reg8>(selector), static_cast<uint8_t>(value));
}

uint16_t CPU::ReadStackWord(uint16_t stackPointer)
{
	const uint8_t lo = m_memoryUnit[PhysAddr(m_registerFile.SS(), stackPointer)].value();
	const uint8_t hi = m_memoryUnit[PhysAddr(m_registerFile.SS(), stackPointer + 1)].value();

	return static_cast<uint16_t>((hi << 8) | lo);
}

void CPU::ShortJump(int8_t offset)
{
	m_IP = m_IP + offset;
}

void CPU::Push16(uint16_t value)
{
	const uint16_t sp = static_cast<uint16_t>(m_registerFile.SP() - 2);
	m_registerFile.Set16(Reg16::SP, sp);

	const uint8_t lo = static_cast<uint8_t>(value & 0xFF);
	const uint8_t hi = static_cast<uint8_t>(value >> 8);

	m_memoryUnit[PhysAddr(m_registerFile.SS(), sp)] = lo;
	m_memoryUnit[PhysAddr(m_registerFile.SS(), sp + 1)] = hi;
}

uint16_t CPU::Pop16()
{
	const uint16_t word = ReadStackWord(m_registerFile.SP());
	m_registerFile.SP() += 2;
	return word;
}

template<typename RegType>
void CPU::StackOperation(RegType regType, bool isPush)
{
	if (isPush)
	{
		const uint16_t value = m_registerFile.Get16(regType);
		Push16(value);
	}
	else
	{
		const uint16_t sp = static_cast<uint16_t>(m_registerFile.SP());
		const uint16_t word = Pop16();
		m_registerFile.Set16(static_cast<Reg16>(regType), word);
	}
	++m_IP;
}
