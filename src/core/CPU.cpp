#include "Core/CPU.h"

CPU::CPU(InterruptController& interruptController) :
	m_interruptController{ interruptController }
{
	Reset();
}

uint20_t CPU::PhysAddr() const noexcept
{
	return uint20_t(static_cast<uint32_t>((m_registerFile.CS() << 4) + m_IP));
}

uint8_t CPU::Fetch8()
{
	assert(m_memoryUnit[PhysAddr()].has_value());
	++m_IP;
	return m_memoryUnit[PhysAddr()].value();
}

uint16_t CPU::Fetch16()
{
	assert(m_memoryUnit[PhysAddr()].has_value());
	uint8_t lo = m_memoryUnit[PhysAddr()].value();
	++m_IP;

	assert(m_memoryUnit[PhysAddr()].has_value());
	uint8_t hi = m_memoryUnit[PhysAddr()].value();
	++m_IP;

	return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(lo);
}

void CPU::Step()
{
	if (m_haltFlag) return;

	switch (m_IR)
	{
	case 0x90: // NOP, 1by
	{
		break;
	}
	case 0xF4: // HLT 1by
	{
		m_haltFlag = true;
		break;
	}

	// MOV reg16, imm16 3by
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	case 0xBC:
	case 0xBD:
	case 0xBE:
	case 0xBF:
	{
		const uint8_t selector = m_IR & 0b111;
		const uint16_t imm16 = Fetch16();
		m_registerFile.Set16(selector, imm16);
		break;
	}

	// MOV reg8_lo, imm8
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	{
		const uint8_t selector = m_IR & 0b111;
		const uint8_t imm8 = Fetch8();
		m_registerFile.SetLo(selector, imm8);
		break;
	}

	// MOV reg8_hi, imm8
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	{
		const uint8_t selector = m_IR & 0b111;
		const uint8_t imm8 = Fetch8();
		m_registerFile.SetHi(selector, imm8);
		break;
	}
	default:
	{
		std::cout << "Hi from default\n";
		std::cout << std::hex << static_cast<int>(m_IR) << '\n';
		break;
	}
	}
	UpdateIR();
	//if (m_haltFlag || m_memoryUnit.IsMemoryEmpty()) return;

	//switch (m_IR)
	//{

	///* Load & Store instructions */

	//case 0x01: // LDA_IM 2by
	//{
	//	m_registerFile.A() = m_memoryUnit[++m_PC].value();
	//	break;
	//}

	//case 0x02: // LDA_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	m_registerFile.A() = m_memoryUnit.Read(address);
	//	break;
	//}

	//case 0x03: // LDA_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	m_registerFile.A() = m_registerFile.Read(selector);
	//	break;
	//}

	//case 0x04: // STA_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_memoryUnit[address] = m_registerFile.A();
	//	break;
	//}

	//case 0x05: // LDB_IM 2by
	//{
	//	m_registerFile.B() = m_memoryUnit.Read(++m_PC);
	//	break;
	//}

	//case 0x06: // LDB_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_registerFile.B() = m_memoryUnit.Read(address);
	//	break;
	//}

	//case 0x07: // LDB_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	m_registerFile.B() = m_registerFile.Read(selector);
	//	break;
	//}

	//case 0x08: // STB_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_memoryUnit[address] = m_registerFile.B();
	//	break;
	//}

	//case 0x09: // LDC_IM 2by
	//{
	//	m_registerFile.C() = m_memoryUnit.Read(++m_PC);
	//	break;
	//}

	//case 0x0A: // LDC_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_registerFile.C() = m_memoryUnit.Read(address);
	//	break;
	//}

	//case 0x0B: // LDC_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	m_registerFile.C() = m_registerFile.Read(selector);
	//	break;
	//}

	//case 0x0C: // STC_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_memoryUnit[address] = m_registerFile.C();
	//	break;
	//}

	//case 0x0D: // LDD_IM 2by
	//{
	//	m_registerFile.D() = m_memoryUnit.Read(++m_PC);
	//	break;
	//}

	//case 0x0E: // LDD_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_registerFile.D() = m_memoryUnit.Read(address);
	//	break;
	//}

	//case 0x0F: // LDD_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	m_registerFile.D() = m_registerFile.Read(selector);
	//	break;
	//}

	//case 0x10: // STD_DIR 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;
	//	m_memoryUnit[address] = m_registerFile.D();
	//	break;
	//}

	//case 0x11: // LDA_IND 1by
	//{
	//	uint16_t BC = (m_registerFile.B() << 8) | m_registerFile.C();
	//	m_registerFile.A() = m_memoryUnit.Read(BC);
	//	break;
	//}

	//case 0x12: // STA_IND 1by
	//{
	//	uint16_t BC = (m_registerFile.B() << 8) | m_registerFile.C();
	//	m_memoryUnit[BC] = m_registerFile.A();
	//	break;
	//}

	//case 0x13: // LDW_IM 3by
	//{
	//	m_registerFile.B() = m_memoryUnit.Read(++m_PC);
	//	m_registerFile.C() = m_memoryUnit.Read(++m_PC);
	//	break;
	//}

	//case 0x14: // INCW 1by
	//{
	//	uint16_t BC = (m_registerFile.B() << 8) | m_registerFile.C();
	//	++BC;

	//	m_registerFile.B() = (BC >> 8) & 0xFF;
	//	m_registerFile.C() = BC & 0xFF;
	//	break;
	//}

	///* Logical operations */

	//case 0x20: // ADD_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::Add(m_registerFile.A(), operand, false);
	//	
	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}	

	//case 0x21: // ADD_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Add(m_registerFile.A(), reg, false);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x22: // ADC_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::Add(m_registerFile.A(), operand, true);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x23: // ADC_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Add(m_registerFile.A(), reg, true);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x24: // SUB_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::Sub(m_registerFile.A(), operand, false);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x25: // SUB_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Sub(m_registerFile.A(), reg, false);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x26: // SBC_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::Sub(m_registerFile.A(), operand, true);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x27: // SBC_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Sub(m_registerFile.A(), reg, true);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x28: // INC 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Inc(reg);

	//	m_registerFile.Write(selector, r.value);
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x29: // DEC 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Dec(reg);

	//	m_registerFile.Write(selector, r.value);
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x2A: //MUL_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::Mul(m_registerFile.A(), operand);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x2B: //MUL_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Mul(m_registerFile.A(), reg);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	///* Logical operations*/

	//case 0x30: // AND_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::AND(m_registerFile.A(), operand);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x31: // AND_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::AND(m_registerFile.A(), reg);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x32: // OR_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::OR(m_registerFile.A(), operand);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x33: // OR_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::OR(m_registerFile.A(), reg);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x34: // XOR_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::XOR(m_registerFile.A(), operand);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x35: // XOR_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::XOR(m_registerFile.A(), reg);

	//	m_registerFile.A() = r.value;
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x36: //NOT 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::NOT(reg);
	//	
	//	reg = r.value;
	//	m_registerFile.Write(selector, reg);
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x37: //SHL 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::ShiftLeft(reg);

	//	reg = r.value;
	//	m_registerFile.Write(selector, reg);
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x38: //SHR 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::ShiftLeft(reg);

	//	reg = r.value;
	//	m_registerFile.Write(selector, reg);
	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	///* Compare instructions */

	//case 0x40: // CMP_IM 2by
	//{
	//	uint8_t operand = m_memoryUnit.Read(++m_PC);
	//	Result r = ALU::Compare(m_registerFile.A(), operand);

	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	//case 0x41: // CMP_REG 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);
	//	Result r = ALU::Compare(m_registerFile.A(), reg);

	//	m_statusRegister.UpdateFrom(r);
	//	break;
	//}

	///* Jump/Branch instructions */

	//case 0x50: // JMP 3by
	//{
	//	uint8_t high = m_memoryUnit.Read(++m_PC);
	//	uint8_t low = m_memoryUnit.Read(++m_PC);
	//	uint16_t address = (high << 8) | low;

	//	m_PC = address;
	//	SetInstructionRegister();
	//	return;
	//}

	//case 0x51: // JZ 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (m_statusRegister.Zero())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x52: // JNZ 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (!m_statusRegister.Zero())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x53: // JC 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (m_statusRegister.Carry())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x54: // JNC 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (!m_statusRegister.Carry())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x55: // JN 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (m_statusRegister.Negative())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x56: // JNN 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (!m_statusRegister.Negative())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x57: // JO 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (m_statusRegister.Overflow())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	//case 0x58: // JNO 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	if (!m_statusRegister.Overflow())
	//	{
	//		m_PC = address;
	//		SetInstructionRegister();
	//		return;
	//	}
	//	break;
	//}

	///* Stack instructions */

	//case 0x60: // PUSH 2by
	//{
	//	const uint8_t selector = m_memoryUnit[++m_PC].value();
	//	const uint8_t reg = m_registerFile.Read(selector);

	//	m_memoryUnit.Write(m_SP--, reg);
	//	break;
	//}

	//case 0x61: // POP 2by
	//{
	//	uint8_t selector = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(selector);

	//	reg = m_memoryUnit.Read(++m_SP);
	//	m_registerFile.Write(selector, reg);
	//	break;
	//}

	//case 0x62: // CALL 3by
	//{
	//	uint16_t address = ComputeAddress(m_PC);
	//	m_PC += 2;

	//	m_memoryUnit.Write(m_SP--, (m_PC >> 8) & 0xFF);
	//	m_memoryUnit.Write(m_SP--, m_PC & 0xFF);
	//	m_PC = address;
	//	break;
	//}

	//case 0x63: // RET 1by
	//{
	//	uint8_t low = m_memoryUnit.Read(++m_SP);
	//	uint8_t high = m_memoryUnit.Read(++m_SP);
	//	m_PC = (high << 8) | low;
	//	break;
	//}

	///* Misc instructions */

	//case 0x00: // NOP 1by
	//{
	//	break;
	//}

	//case 0x70: // MOV 3by
	//{
	//	uint8_t destination = m_memoryUnit.Read(++m_PC);
	//	uint8_t source = m_memoryUnit.Read(++m_PC);
	//	uint8_t reg = m_registerFile.Read(source);

	//	m_registerFile.Write(destination, reg);
	//	break;
	//}

	//case 0xF3: // EI 1by
	//{
	//	m_interruptController.EnableInterrupts(true);
	//	break;
	//}

	//case 0xF8: // DI 1by
	//{
	//	m_interruptController.EnableInterrupts(false);
	//	break;
	//}

	//case 0xF9: // RETI 1by
	//{
	//	const uint8_t hi = m_memoryUnit[m_SP++].value();
	//	const uint8_t lo = m_memoryUnit[m_SP++].value();
	//	m_PC = (hi << 8) | lo;
	//	break;
	//}

	//case 0xFF: // HLT 1by
	//{
	//	m_haltFlag = true;
	//	break;
	//}
	//
	// 
	// 
	//}
	//Fetch();
}

void CPU::Reset()
{
	m_registerFile.Reset();
	m_statusRegister.Reset();
	m_interruptController.EnableInterrupts(false);
	m_haltFlag = false;

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

void CPU::HandleInterrupt()
{
	if (auto result = m_interruptController.PollPendingInterrupt(); result.has_value())
	{
		const uint16_t line = result.value();

		const uint16_t hiPC = (m_IP >> 8) & 0xFF;
		const uint16_t loPC = m_IP & 0xFF;

		// save PC address on the stack
		m_memoryUnit[m_registerFile.SP()--] = hiPC;
		m_memoryUnit[m_registerFile.SP()--] = loPC;

		const uint16_t vectorAddress = line * 2;
		m_IP = ComputeAddress(vectorAddress);
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
	m_IR = m_memoryUnit[PhysAddr()].value();
}

void CPU::UpdateIR()
{
	if (m_haltFlag) return;

	m_IR = m_memoryUnit[PhysAddr()].value();
	++m_IP;
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

const StatusRegister& CPU::GetStatusRegister() const noexcept
{
	return m_statusRegister;
}

uint16_t CPU::ComputeAddress(uint16_t address)
{
	uint16_t high = m_memoryUnit[++address].value();
	uint16_t low = m_memoryUnit[++address].value();
	return (high << 8) | low;
}
