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

	m_segmentOverride = DecodeSegmentOverride();

	if ((m_IR & 0xC0) == 0x00 && (m_IR & 0b111) < 6) ExecALU();
	/*
	^	opcodes:	00 -> 05 (ADD)
					08 -> 0D (OR)
					10 -> 15 (ADC)
					18 -> 1D (SBB)
					20 -> 25 (AND)
					28 -> 2D (SUB)
					30 -> 35 (XOR)
					38 -> 3D (CMP)
		total: 48 instructions
	*/
	else if ((m_IR & 0xE6) == 0x06)												 ExecPushPopSeg(); // opcodes: {06, 07, 0E, 0F, 16, 17, 1E, 1F} (8 instr)
	else if ((m_IR & 0xF0) == 0x40)												 ExecINCDEC(); // opcodes: 40 -> 4F (16 instr)
	else if ((m_IR & 0xF0) == 0x50)												 ExecPushPopGP(); // opcodes: 50 -> 5F (16 instr) 
	else if ((m_IR & 0xF0) == 0x60 || (m_IR & 0xF0) == 0x70)					 ExecConditionedJump(); // opcodes: 70 -> 7F (16 instr)
	else if ((m_IR & 0xFC) == 0x80)												 ExecGRP1();			// opcodes: 80 -> 83 (4 instr)
	else if ((m_IR & 0xFE) == 0x84)												 ExecTESTModRM();		// opcodes: {84, 85} (2 instr)
	else if ((m_IR & 0xFE) == 0x86)												 ExecXCHGModRM();		// opcodes: {86, 87} (2 instr)
	else if ((m_IR & 0xFC) == 0x88)												 ExecMOVModRM();		// opcodes: 88 -> 8B (4 instr)
	else if ((m_IR & 0xF8) == 0x90)												 ExecXCHG();			// opcodes: 90 -> 97 (8 instr)
	else if ((m_IR & 0xFE) == 0x98)												 ExecConvert();			// opcodes: 98, 99 (CBW, CWD)
	else if ((m_IR & 0xFE) == 0x9C)												 ExecPushPopFlags();	// opcodes: 9C, 9D (PUSHF, POPF) 
	else if ((m_IR & 0xFE) == 0x9E)												 ExecStoreRegIntoFlags(); // opcodes: 9E, 9F (SAHF, LAHF)
	else if ((m_IR & 0xFC) == 0xA0)												 ExecMOVAccumulator();	// opcodes: A0 -> A3 (4 instr)
	else if ((m_IR & 0xFE) == 0xA4)												 ExecMOVS();			// opcodes: {A4, A5} (2 instr)
	else if ((m_IR & 0xFE) == 0xA6)												 ExecCMPS();			// opcodes: {A6, A7} (2 instr)
	else if ((m_IR & 0xFE) == 0xA8)											     ExecTEST();			// opcodes: {A8, A9} (2 instr)
	else if ((m_IR & 0xFE) == 0xAA)												 ExecSTOS();			// opcodes: {AA, AB} (2 instr)
	else if ((m_IR & 0xFE) == 0xAC)												 ExecLODS();			// opcodes: {AC, AD} (2 instr)
	else if ((m_IR & 0xFE) == 0xAE)												 ExecSCAS();			// opcodes: {AE, AF} (2 instr)
	else if ((m_IR & 0xF0) == 0xB0)												 ExecMOVImm();			// opcodes: B0 -> BF (16 instr)
	else if ((m_IR & 0xFE) == 0xC0)											     ExecRet();				// opcodes: {C2, C3} (RET Iw, RET) Note: {C0, C1} are aliases for {C2, C3}
	else if ((m_IR & 0xFE) == 0xC4)												 ExecDoubleWord();		// opcodes: {C4, C5} (LES, LDS)
	else if ((m_IR & 0xFE) == 0xC6)												 ExecMOVRMImm();		// opcodes: {C6, C7} (MOV Eb/Ev, Ib/Iv)
	else if ((m_IR & 0xFE) == 0xC8)												 ExecRetFar();			// opcodes: CA, CB (RETF Iw, RETF) Note: {C8, C9} are aliases for {C8, C9}
	else if ((m_IR & 0xFE) == 0xCC)												 ExecInterrupt(Fetch8()); // opcodes: {CC, CD} (INT)
	else if ((m_IR & 0xFE) == 0xD0)										         ExecGRP2();			// opcodes: D0 -> D3 (4 instr for GRP2)
	else if ((m_IR & 0xFE) == 0xD8)												 ExecESC();				// opcodes: D8 -> DF (8 instr)
	else if ((m_IR & 0xFC) == 0xE0)												 ExecLoop();			// opcodes: E0 -> E3 (LOOPNZ / LOOPNE, LOOPE / LOOPZ, LOOP, JCXZ)
	else if ((m_IR & 0xFE) == 0xFC)												 ExecClearStoreFlags(); // opcodes: F8 -> FD (CLC, STC, CLI, STI, CLD, STD)
	else if ((m_IR & 0xFE) == 0xFE)												 ExecGRP4_GRP5();		// opcodes: FE, FF (GRP4 & GRP5)
	else switch (m_IR)
	{
	case 0x27:  // DAA (Decimal adjust After Addition)
	{
		DecimalAdjust(false);

		++m_IP;
		break;
	}
	case 0x2F:			// DAS (Decimal adjust After Subtraction)
	{
		DecimalAdjust(true);

		++m_IP;
		break;
	}
	case 0x37:			// AAA (ASCII Adjust After Addition)
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
	case 0x8C:			// MOV Ew, Sw (MOV for segment registers)
	case 0x8E:			// MOV Sw, Ew
	{
		const bool isSegDest = ((m_IR >> 1) & 1) != 0;
		const ModRM modRM = DecodeModRM();
		const RMOperand rm = ResolveRM(modRM);
		uint16_t value{};

		if (isSegDest)
		{
			value = ReadRM(rm, true);
			WriteReg(modRM.Reg(), value, true, false);
		}
		else
		{
			value = m_registerFile.Get16(static_cast<SegmentReg>(modRM.Reg()));
			WriteRM(rm, value, true, true);
		}

		++m_IP;
		break;
	}
	case 0x8D:			// LEA Gv, M (Load Effective Address)
	{
		const ModRM modRM = DecodeModRM();
		const RMOperand rm = ResolveRM(modRM);

		m_registerFile.Set16(static_cast<Reg16>(modRM.Reg()), rm.offset);
		++m_IP;
		break;
	}
	case 0x8F:			// POP Ev
	{
		const ModRM modRM  = DecodeModRM();
		const RMOperand rm = ResolveRM(modRM);
		++m_IP;

		if (modRM.Reg() != 0b000)
		{
			Logger::AddInfoMessage(std::format("Invalid ModRM Reg field: {}", modRM.Reg()));
			break;
		}

		const uint16_t value = Pop16();
		WriteRM(rm, value, true, true);
		break;
	}
	case 0x9A:			// CALL Ap (Address pointer)
	{
		const uint16_t offset =  Fetch16();
		const uint16_t segment = Fetch16();
		++m_IP;

		Push16(m_registerFile.CS(), true);
		Push16(m_IP, true);

		m_IP = offset;
		m_registerFile.CS() = segment;

		break;
	}
	case 0x9B:			// WAIT
	{
		if (!m_TESTPin)
		{
			++m_IP;
		}

		break;
	}
	case 0xCE:		// INTO (INT if OF = 1)
	{
		if (m_statusRegister.Overflow())
		{
			ExecInterrupt(4u);
		}

		break;
	}
	case 0xCF:		// IRET (Interrupt Return)
	{
		m_IP = Pop16();
		m_registerFile.CS() = Pop16();
		m_statusRegister.SetFlags(Pop16());

		break;
	}
	case 0xD4:			// AAM (ASCII Adjust After Multiplication)
	{
		const uint8_t base = Fetch8();
		m_registerFile.Set8(Reg8::AH, static_cast<uint8_t>(m_registerFile.AL() / base));
		m_registerFile.Set8(Reg8::AL, static_cast<uint8_t>(m_registerFile.AL() % base));

		++m_IP;
		break;
	}
	case 0xD5:			// AAD (ASCII Adjust After Division)
	{
		const uint8_t base = Fetch8();
		m_registerFile.Set8(Reg8::AL, static_cast<uint8_t>(m_registerFile.AH() * base) + m_registerFile.AL());
		m_registerFile.Set8(Reg8::AH, 0u);

		++m_IP;
		break;
	}
	case 0xD6:			// SALC (1by)
	{
		if (m_statusRegister.Carry()) m_registerFile.Set8(Reg8::AL, 0xFF);
		else						  m_registerFile.Set8(Reg8::AL, 0x00);

		++m_IP;
		break;
	}
	case 0xD7:			// XLAT (1by)
	{
		const uint16_t offset = static_cast<uint16_t>(m_registerFile.BX(), m_registerFile.AL());
		if (const auto result = m_memoryUnit[PhysAddr(m_registerFile.DS(), offset)]; result.has_value())
		{
			m_registerFile.Set8(Reg8::AL, result.value());
		}
		++m_IP;
		break;
	}
	case 0xE8:			// CALL Jv (Near Call)
	{
		const int16_t disp = static_cast<int16_t>(Fetch16());
		++m_IP;
		Push16(m_IP, true);
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
		const ModRM modRM = DecodeModRM();
		const RMOperand rm = ResolveRM(modRM);

		const bool word  = (form & 1)		!= 0;		// 0th bit: 16-bits, else 8-bits
		const bool toReg = (form & 0b10)	!= 0;		// if bit 1 is true, destination is reg

		const uint16_t regValue = ReadReg(modRM.Reg(), word);
		const uint16_t rmValue  = ReadRM(rm, word);

		const uint16_t destination = toReg ? regValue : rmValue;
		const uint16_t source      = toReg ? rmValue : regValue;

		const Result r = ALU::Execute(operation, destination, source, word, m_statusRegister.Carry());
		m_statusRegister.UpdateFrom(r);

		if (operation != 0b111) // not CMP
		{
			if (toReg) WriteReg(modRM.Reg(), r.value, word, true);
			else	   WriteRM(rm, r.value, word, true);
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
			if (word) m_registerFile.Set16(Reg16::AX, r.value);
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
	const bool push = (m_IR & 1) == 0;

	if (push)
	{
		const uint16_t sp = static_cast<uint16_t>(m_registerFile.Get16(Reg16::SP) - 2);
		const uint16_t flags = m_statusRegister.GetFlags();
		m_registerFile.Set16(Reg16::SP, sp);

		m_memoryUnit[PhysAddr(m_registerFile.SS(), sp)] = static_cast<uint8_t>(flags & 0xFF);
		m_memoryUnit[PhysAddr(m_registerFile.SS(), static_cast<uint16_t>(sp + 1))] = static_cast<uint8_t>(flags >> 8);
	}
	else
	{
		const uint16_t sp = static_cast<uint16_t>(m_registerFile.SP());
		const uint16_t flags = ReadStackWord(sp);
		m_registerFile.Set16(Reg16::SP, sp + 2);
		m_statusRegister.SetFlags(flags);
	}
	++m_IP;
}

void CPU::ExecMOVImm()
{
	const bool word = (m_IR >> 3) & 1; // word == 1 -> full reg / word == 0 -> Hi / Lo
	const uint8_t selector = m_IR & 0b111;

	if (word) m_registerFile.Set16(static_cast<Reg16>(selector), Fetch16());
	else      m_registerFile.Set8(static_cast<Reg8>(selector), Fetch8());
	
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

void CPU::ExecMOVAccumulator()
{
	const bool word = (m_IR & 1) == 1;
	const bool copyFromMem = (m_IR >> 1) == 0;
	const uint16_t reg = word ? m_registerFile.AX() : m_registerFile.AL();
	const uint16_t offset = word ? Fetch16() : Fetch8();

	if (!word) ++m_IP;

	if (copyFromMem)
	{
		word ? m_registerFile.Set16(Reg16::AX, offset)
			 : m_registerFile.Set8(Reg8::AL, static_cast<uint8_t>(offset));
	}
	else
	{
		m_memoryUnit[PhysAddr(m_registerFile.CS(), offset)] = static_cast<uint8_t>(reg & 0xFF); // lo byte
		if (word)
		{
			m_memoryUnit[PhysAddr(m_registerFile.CS(), static_cast<uint16_t>(offset + 1))] = static_cast<uint8_t>(reg >> 8); // hi byte
		}
	}
	++m_IP;
}

void CPU::ExecMOVModRM()
{
	const ModRM modRM  = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);

	const bool generalReg = true;
	const bool word  = (m_IR & 1)    != 0; // bit 0 is 1, then operate on 16-bits, else 8-bits
	const bool toReg = (m_IR & 0b10) != 0; 

	const uint16_t value = toReg ? ReadRM(rm, word) : ReadReg(modRM.Reg(), word);

	if (toReg) WriteReg(modRM.Reg(), value, word, generalReg);
	else WriteRM(rm, value, word, generalReg);

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
		Logger::AddInfoMessage(std::format("Unknown opcode: {}", m_IR));
		break;
	}
	}
	++m_IP;
}

void CPU::ExecGRP1()
{
	const bool word  = (m_IR & 1) != 0;
	const bool signExtend = (m_IR & 0b10) != 0 && word;
	const ModRM modRM  = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);

	const uint16_t firstOperand = ReadRM(rm, word);
	uint16_t secondOperand{};

	if (!word)				secondOperand = Fetch8();
	else if (signExtend)	secondOperand = static_cast<uint16_t>(static_cast<int8_t>(Fetch8()));
	else                    secondOperand = Fetch16();

	const Result r = ALU::Execute(modRM.Reg(), firstOperand, secondOperand, word, m_statusRegister.Carry());
	m_statusRegister.UpdateFrom(r);

	if (modRM.Reg() != 0b111) // Not CMP
	{
		WriteRM(rm, r.value, word, true);
	}
	++m_IP;
}

void CPU::ExecGRP2()
{
	const bool word = (m_IR & 1) != 0;
	const bool countCL = ((m_IR >> 1) & 1) != 0;
	const ModRM modRM = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);
	const uint8_t count = countCL ? m_registerFile.CL() : 1u;

	const Result r = ALU::Shift(modRM.Reg(), ReadRM(rm, word), count, word, m_statusRegister.Carry());
	WriteRM(rm, r.value, word, true);
	m_statusRegister.UpdateFrom(r);

	++m_IP;
}

void CPU::ExecGRP4_GRP5()
{
	const bool word = (m_IR & 1) == 1;
	const ModRM modRM = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);
	++m_IP;

	const uint16_t value = ReadRM(rm, word);
	Result r;

	switch (modRM.Reg())
	{
	case 0b000: // INC
	{
		r = ALU::INC(value, word);
		WriteRM(rm, r.value, word, true);
		m_statusRegister.UpdateFrom(r);
		return;
	}
	case 0b001: // DEC 
	{
		r = ALU::DEC(value, word);
		WriteRM(rm, r.value, word, true);
		m_statusRegister.UpdateFrom(r);
		return;
	}
	case 0b010: // CALL Ev
	{
		Push16(m_IP, word);
		m_IP = value; // Approximation: Will be updated when hardware test will be made
		return;
	}
	case 0b011: // CALL Mp
	{
		if (rm.isReg)
		{
			Logger::AddInfoMessage(std::format("Unimplemented RM operand"));
			return;
		}

		const uint16_t offset = ReadMem16(rm.segment, rm.offset);
		const uint16_t segment = ReadMem16(rm.segment, static_cast<uint16_t>(offset + 2));

		Push16(m_registerFile.CS(), true);
		Push16(m_IP, true);

		m_IP = offset;
		m_registerFile.CS() = segment;
		return;
	}
	case 0b100: // JMP Ev
	{
		m_IP = value; // Approximation: Will be updated when hardware test will be made
		return;
	}
	case 0b101: // JMP  Mp
	{
		if (rm.isReg)
		{
			Logger::AddInfoMessage(std::format("Unimplemented RM operand"));
			return;
		}

		const uint16_t offset  = ReadMem16(rm.segment, rm.offset);
		const uint16_t segment = ReadMem16(rm.segment, static_cast<uint16_t>(rm.offset + 2));

		m_IP = offset;
		m_registerFile.CS() = segment;
		return;
	}
	case 0b110: // PUSH (Eb, Ev)
	case 0b111: // Alias for 0b110
	{
		Push16(value, word);
		return;
	}
	}
}

void CPU::ExecTEST()
{
	const bool word = (m_IR & 1) != 0;
	const uint16_t value = word ? Fetch16() : Fetch8();
	const uint16_t reg = word ? m_registerFile.AX() : m_registerFile.AL();

	const Result r = ALU::AND(value, reg, word);
	m_statusRegister.UpdateFrom(r);
	++m_IP;
}

void CPU::ExecTESTModRM()
{
	const bool word = (m_IR & 1) != 0;
	const ModRM modRM = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);

	const uint16_t operand = ReadRM(rm, word);
	const uint16_t reg = ReadReg(modRM.Reg(), word);

	const Result r = ALU::AND(operand, reg, word);
	m_statusRegister.UpdateFrom(r);
	++m_IP;
}

void CPU::ExecMOVS()
{
	const bool word = (m_IR & 1) != 0;
	const uint8_t inc = word ? 2u : 1u;
	const uint16_t value = word ? ReadMem16(m_registerFile.DS(), m_registerFile.SI())
								: ReadMem8(m_registerFile.DS(), m_registerFile.SI());

	if (word) WriteMem16(m_registerFile.ES(), m_registerFile.DI(), value);
	else      WriteMem8(m_registerFile.ES(), m_registerFile.DI(), static_cast<uint8_t>(value));

	UpdateDirection(m_statusRegister.Directional(), true, true, inc);
	++m_IP;
}

void CPU::ExecCMPS()
{
	const bool word = (m_IR & 1) != 0;
	const uint8_t inc = word ? 2u : 1u;
	const uint16_t source = word ? ReadMem16(m_registerFile.DS(), m_registerFile.SI())
								 : ReadMem8(m_registerFile.DS(), m_registerFile.SI());

	const uint16_t dest = word ? ReadMem16(m_registerFile.ES(), m_registerFile.DI())
								: ReadMem8(m_registerFile.ES(), m_registerFile.DI());

	const Result r = ALU::SUB(source, dest, false, word);
	m_statusRegister.UpdateFrom(r);
	UpdateDirection(m_statusRegister.Directional(), true, true, inc);

	++m_IP;
}

void CPU::ExecSTOS()
{
	const bool word = (m_IR & 1) != 0;
	const uint8_t inc = word ? 2u : 1u;
	const uint16_t value = word ? m_registerFile.AX()
								: m_registerFile.AL();

	if (word) WriteMem16(m_registerFile.ES(), m_registerFile.DI(), value);
	else      WriteMem8(m_registerFile.ES(), m_registerFile.DI(), static_cast<uint8_t>(value));

	UpdateDirection(m_statusRegister.Directional(), false, true, inc);
	++m_IP;
}

void CPU::ExecLODS()
{
	std::cout << "lods\n";
	const bool word = (m_IR & 1) != 0;
	const uint8_t inc = word ? 2u : 1u;
	const uint16_t value = word ? ReadMem16(m_registerFile.DS(), m_registerFile.SI())
								: ReadMem8(m_registerFile.DS(), m_registerFile.SI());
	std::cout << "value: " << std::hex << static_cast<int>(value) << '\n';

	if (word) m_registerFile.AX() = value;
	else
	{
		std::cout << "byte\n";
		m_registerFile.Set8(Reg8::AL, static_cast<uint8_t>(value));
	}

	UpdateDirection(m_statusRegister.Directional(), true, false, inc);
	++m_IP;
}

void CPU::ExecSCAS()
{
	const bool word = (m_IR & 1) != 0;
	const uint8_t inc = word ? 2u : 1u;
	const uint16_t reg = word ? m_registerFile.AX()
							  : m_registerFile.AL();

	const uint16_t operand = word ? ReadMem16(m_registerFile.ES(), m_registerFile.DI())
								  : ReadMem8(m_registerFile.ES(), m_registerFile.DI());
	const Result r = ALU::SUB(reg, operand, false, word);
	m_statusRegister.UpdateFrom(r);

	UpdateDirection(m_statusRegister.Directional(), false, true, inc);
	++m_IP;
}

void CPU::ExecDoubleWord()
{
	std::cout << "double word\n";
	const bool dataSeg = (m_IR & 1) != 0;
	const ModRM modRM = DecodeModRM();

	if (modRM.RM() == 0b11)
	{
		Logger::AddInfoMessage(std::format("Invalid RM field: {}", modRM.RM()));
		++m_IP;
		return;
	}

	const RMOperand rm = ResolveRM(modRM);
	m_registerFile.Set16(static_cast<Reg16>(modRM.Reg()), ReadMem16(rm.segment, rm.offset));

	if (dataSeg) m_registerFile.DS() = ReadMem16(rm.segment, static_cast<uint16_t>(rm.offset + 2));
	else         m_registerFile.ES() = ReadMem16(rm.segment, static_cast<uint16_t>(rm.offset + 2));

	++m_IP;
}

void CPU::ExecMOVRMImm()
{
	const bool word = (m_IR & 1) != 0;
	const ModRM modRM = DecodeModRM();

	if (modRM.Reg() != 0)
	{
		Logger::AddInfoMessage(std::format("Invalid Reg field: {}", modRM.Reg()));
		++m_IP;
		return;
	}

	const RMOperand rm = ResolveRM(modRM);
	const uint16_t imm = word ? Fetch16() : Fetch8();

	WriteRM(rm, imm, word, true);

	++m_IP;
}

void CPU::ExecInterrupt(uint8_t interrupt)
{
	++m_IP;
	Push16(m_statusRegister.GetFlags(), true);
	Push16(m_registerFile.CS(), true);
	Push16(m_IP, true);

	m_statusRegister.SetFlagOff(Utils::Constants::IF_MASK); 
	m_statusRegister.SetFlagOff(Utils::Constants::TF_MASK);
	/* 
		TODO: Transfer control to interrupt procedure(will be implemented when IVT will be made).
		m_IP = IVT[interrupt * 4];
		m_registerFile.CS() = IVT[interrupt * 4 + 2];
	* */
}

void CPU::ExecESC()
{
	/* Intel 8087 coprocessor instructions */

	const ModRM modRM = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);
	++m_IP;
}

void CPU::DecimalAdjust(bool isSubtract)
{
	const uint8_t oldAL = m_registerFile.AL();
	const bool oldCF = m_statusRegister.Carry();

	uint8_t al = oldAL;
	bool carry = false;
	bool aux = false;

	if ((oldAL & 0x0F) > 9 || m_statusRegister.Auxiliary())
	{
		const int result = oldAL + (isSubtract ? -6 : 6);
		carry = oldCF || result > 0xFF || result < 0;
		al = static_cast<uint8_t>(result);
		aux = true;
	}

	if (oldAL > 0x99 || oldCF)
	{
		al = static_cast<uint8_t>(al + (isSubtract ? -0x60 : 0x60));
		carry = true;
	}

	m_registerFile.Set8(Reg8::AL, al);

	m_statusRegister.SetFlag(Utils::Constants::CF_MASK, carry);
	m_statusRegister.SetFlag(Utils::Constants::AF_MASK, aux);
	m_statusRegister.SetFlag(Utils::Constants::ZF_MASK, al == 0);
	m_statusRegister.SetFlag(Utils::Constants::SF_MASK, (al & 0x80) != 0);
	m_statusRegister.SetFlag(Utils::Constants::PF_MASK, (std::popcount(al) % 2) == 0);
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
		result = static_cast<bool>(m_statusRegister.Overflow()) ^ negative;
		break;
	}
	case 0b001:		// JB & JNB
	{
		result = static_cast<bool>(m_statusRegister.Carry()) ^ negative;
		break;
	}
	case 0b010:		// JZ & JNZ
	{
		result = static_cast<bool>(m_statusRegister.Zero()) ^ negative;
		break;
	}
	case 0b011:		// JBE (CF = 1 or ZF = 1) & JA (CF = 0 and ZF = 0)
	{
		result = (m_statusRegister.Carry() || m_statusRegister.Zero()) ^ negative;
		break;
	}
	case 0b100:		// JS & JNS
	{
		result = static_cast<bool>(m_statusRegister.Sign()) ^ negative;
		break;
	}
	case 0b101:		// JPE & JPO
	{
		result = static_cast<bool>(m_statusRegister.Parity()) ^ negative;
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
			static_cast<bool>(m_statusRegister.Zero())) ^ negative;
		break;
	}
	}

	if (result)
	{
		ShortJump(offset);
	}
}

void CPU::ExecXCHG()
{
	const uint8_t selector = m_IR & 0b111;
	std::swap(m_registerFile.AX(),
		m_registerFile.Get16(static_cast<Reg16>(selector)));
	++m_IP;
}

void CPU::ExecXCHGModRM()
{
	const bool generalReg = true;
	const bool word = (m_IR & 1) != 0;
	const ModRM modRM = DecodeModRM();
	const RMOperand rm = ResolveRM(modRM);

	const uint16_t operand = ReadRM(rm, word);
	const uint16_t reg = ReadReg(modRM.Reg(), word);

	WriteRM(rm, reg, word, generalReg);
	WriteReg(modRM.Reg(), operand, word, generalReg);
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
	m_segmentOverride = SegmentOverride::NONE;

	m_IP = m_memoryUnit.GetStartAddress();

	if (IsPrefixOverride(m_memoryUnit[m_IP].value_or(0x00)))
	{
		++m_IP;
		m_segmentOverride = DecodeSegmentOverride();

	}
	m_IR = m_memoryUnit[m_IP].value_or(0x00);
}

ModRM CPU::DecodeModRM()
{
	ModRM modRM{ Fetch8() };
	return modRM;
}

uint16_t CPU::DecodeEffectiveAddress(uint8_t rm, uint8_t mod)
{
	uint16_t effectiveAddress{};

	switch (rm) 
	{
	case 0b000:		effectiveAddress = static_cast<uint16_t>(m_registerFile.BX() + m_registerFile.SI());	break;
	case 0b001:		effectiveAddress = static_cast<uint16_t>(m_registerFile.BX() + m_registerFile.DI());	break;
	case 0b010:		effectiveAddress = static_cast<uint16_t>(m_registerFile.BP() + m_registerFile.SI());	break;
	case 0b011:		effectiveAddress = static_cast<uint16_t>(m_registerFile.BP() + m_registerFile.DI());	break;
	case 0b100:		effectiveAddress = m_registerFile.SI();													break;
	case 0b101:		effectiveAddress = m_registerFile.DI();													break;
	case 0b110:
	{
		// Exception when mod is 0b00: we return the direct address
		if (mod == 0b00)
		{
			const uint16_t directAddr = Fetch16();
			return directAddr;
		}
		effectiveAddress = m_registerFile.BP();
																											break;
	}
	case 0b111:		effectiveAddress = m_registerFile.BX();													break;
	}

	if (mod == 0b01)
	{
		const int8_t disp = static_cast<int8_t>(Fetch8());
		effectiveAddress += disp;
	}
	else if (mod == 0b10)
	{
		const uint16_t disp = Fetch16();
		effectiveAddress += disp;
	}

	return effectiveAddress;
}

RMOperand CPU::ResolveRM(ModRM modRM)
{
	if (modRM.Mod() == 0b11) // is reg
	{
		return { true, modRM.RM(), 0, 0 };
	}

	const uint16_t offset = DecodeEffectiveAddress(modRM.RM(), modRM.Mod());
	const bool usesBP = modRM.RM()  == 0b010 || modRM.RM()  == 0b011 ||
						(modRM.RM() == 0b110 && modRM.Mod() != 0b00);

	uint16_t segment = usesBP ? m_registerFile.SS() : m_registerFile.DS();

	if (const auto result = ResolveSegment(); result.has_value()) // When segment override occurs
	{
		segment = result.value();
	}

	return { false, 0, segment, offset };
}

void CPU::UpdateDirection(bool directionalFlag, bool updateSource, bool updateDest, uint8_t value) noexcept
{
	if (!directionalFlag)
	{
		if (updateSource) m_registerFile.SI() += value;
		if (updateDest)   m_registerFile.DI() += value;
	}
	else
	{
		if (updateSource) m_registerFile.SI() -= value;
		if (updateDest)   m_registerFile.DI() -= value;
	}
}

SegmentOverride CPU::GetSegmentOverride() const noexcept
{
	return m_segmentOverride;
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
	
	if (IsPrefixOverride(m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value_or(0x00)))
	{
		m_segmentOverride = DecodeSegmentOverride();
		++m_IP;
	}
	m_IR = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value_or(0x00);
}

void CPU::UpdateIR()
{
	if (m_haltFlag) return;

	m_IR = m_memoryUnit[PhysAddr(m_registerFile.CS(), m_IP)].value_or(0x00);
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

void CPU::WriteReg(uint8_t selector, uint16_t value, bool word, bool generalReg)
{
	if (word)
	{
		generalReg  ? m_registerFile.Set16(static_cast<Reg16>(selector), value)
					: m_registerFile.Set16(static_cast<SegmentReg>(selector), value);
		return;
	}
	m_registerFile.Set8(static_cast<Reg8>(selector), static_cast<uint8_t>(value));
}

uint16_t CPU::ReadStackWord(uint16_t stackPointer)
{
	const uint8_t lo = m_memoryUnit[PhysAddr(m_registerFile.SS(), stackPointer)].value_or(0x00);
	const uint8_t hi = m_memoryUnit[PhysAddr(m_registerFile.SS(), static_cast<uint16_t>(stackPointer + 1))].value_or(0x00);

	return static_cast<uint16_t>((hi << 8) | lo);
}

uint8_t CPU::ReadMem8(uint16_t segment, uint16_t offset)
{
	return m_memoryUnit[PhysAddr(segment, offset)].value_or(0);
}

uint16_t CPU::ReadMem16(uint16_t segment, uint16_t offset)
{
	const uint8_t lo = ReadMem8(segment, offset);
	const uint8_t hi = ReadMem8(segment, static_cast<uint16_t>(offset + 1));
	return static_cast<uint16_t>((hi << 8) | lo);
}

void CPU::WriteMem8(uint16_t segment, uint16_t offset, uint8_t value)
{
	m_memoryUnit[PhysAddr(segment, offset)] = value;
}

void CPU::WriteMem16(uint16_t segment, uint16_t offset, uint16_t value)
{
	WriteMem8(segment, offset, value & 0xFF);
	WriteMem8(segment, static_cast<uint16_t>(offset + 1), static_cast<uint8_t>(value >> 8));
}

uint16_t CPU::ReadRM(const RMOperand& operand, bool word)
{
	if (operand.isReg) return ReadReg(operand.reg, word);
	return word ? ReadMem16(operand.segment, operand.offset) : ReadMem8(operand.segment, operand.offset);
}

void CPU::WriteRM(const RMOperand& operand, uint16_t value, bool word, bool generalReg)
{
	if (operand.isReg)
	{
		WriteReg(operand.reg, value, word, generalReg);
		return;
	}
	if (word) WriteMem16(operand.segment, operand.offset, value);
	else	  WriteMem8(operand.segment, operand.offset, static_cast<uint8_t>(value));
}

void CPU::ShortJump(int8_t offset)
{
	m_IP = m_IP + offset;
}

void CPU::Push16(uint16_t value, bool word)
{
	const uint16_t sp = static_cast<uint16_t>(m_registerFile.SP() - 2);
	m_registerFile.Set16(Reg16::SP, sp);

	// Lo byte is always written on the stack
	m_memoryUnit[PhysAddr(m_registerFile.SS(), sp)] = static_cast<uint8_t>(value & 0xFF);

	if (word)
	{
		// Hi byte is only written in word mode
		m_memoryUnit[PhysAddr(m_registerFile.SS(), static_cast<uint16_t>(sp + 1))] = static_cast<uint8_t>(value >> 8);
	}
}

uint16_t CPU::Pop16()
{
	const uint16_t word = ReadStackWord(m_registerFile.SP());
	m_registerFile.SP() += 2;
	return word;
}

bool CPU::IsPrefixOverride(uint8_t value) const noexcept
{
	return value == 0x26
		|| value == 0x2E
		|| value == 0x36
		|| value == 0x3E;
}

SegmentOverride CPU::DecodeSegmentOverride() noexcept
{
	SegmentOverride seg{};
	switch (m_IR) 
	{
	case 0x26: seg = SegmentOverride::ES; break;
	case 0x2E: seg = SegmentOverride::CS; break;
	case 0x36: seg = SegmentOverride::SS; break;
	case 0x3E: seg = SegmentOverride::DS; break;
	default:   return SegmentOverride::NONE;
	}

	++m_IP;
	UpdateIR();
	return seg;
}

std::optional<uint16_t> CPU::ResolveSegment() noexcept
{
	switch (m_segmentOverride)
	{
	case SegmentOverride::NONE: return std::nullopt;
	case SegmentOverride::ES:	return m_registerFile.ES();
	case SegmentOverride::CS:	return m_registerFile.CS();
	case SegmentOverride::SS:	return m_registerFile.SS();
	case SegmentOverride::DS:	return m_registerFile.DS();
	default:					return std::nullopt;
	}
}