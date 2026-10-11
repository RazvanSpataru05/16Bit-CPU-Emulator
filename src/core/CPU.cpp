#include "Core/CPU.h"

CPU::CPU(InterruptController& interruptController) : 
	m_interruptController{ interruptController }
{
	Reset();
}

void CPU::Fetch()
{
	m_IR = m_memoryUnit.Read(m_PC++);
}

void CPU::Step()
{
	if (m_HaltFlag || m_memoryUnit.IsMemoryEmpty()) return;

	Fetch();

	switch (m_IR)
	{

	/* Load/Store instructions */

	case 0x01: // LDA_IM 2by
	{
		m_A = m_memoryUnit.Read(m_PC++);
		break;
	}

	case 0x02: // LDA_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_A = m_memoryUnit.Read(address);
		break;
	}

	case 0x03: // LDA_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		m_A = ReadRegister(selector);
		break;
	}

	case 0x04: // STA_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_memoryUnit[address] = m_A;
		break;
	}

	case 0x05: // LDB_IM 2by
	{
		m_B = m_memoryUnit.Read(m_PC++);
		break;
	}

	case 0x06: // LDB_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_B = m_memoryUnit.Read(address);
		break;
	}

	case 0x07: // LDB_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		m_B = ReadRegister(selector);
		break;
	}

	case 0x08: // STB_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_memoryUnit.Write(address, m_B);
		break;
	}

	case 0x09: // LDC_IM 2by
	{
		m_C = m_memoryUnit.Read(m_PC++);
		break;
	}

	case 0x0A: // LDC_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_C = m_memoryUnit.Read(address);
		break;
	}

	case 0x0B: // LDC_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		m_C = ReadRegister(selector);
		break;
	}

	case 0x0C: // STC_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_memoryUnit.Write(address, m_C);
		break;
	}

	case 0x0D: // LDD_IM 2by
	{
		m_D = m_memoryUnit.Read(m_PC++);
		break;
	}

	case 0x0E: // LDD_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_D = m_memoryUnit.Read(address);
		break;
	}

	case 0x0F: // LDD_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		m_D = ReadRegister(selector);
		break;
	}

	case 0x10: // STD_DIR 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_memoryUnit.Write(address, m_D);
		break;
	}

	case 0x11: // LDA_IND 1by
	{
		uint16_t bc = (m_B << 8) | m_C;
		m_A = m_memoryUnit.Read(bc);
		break;
	}

	case 0x12: // STA_IND 1by
	{
		uint16_t bc = (m_B << 8) | m_C;
		m_memoryUnit.Write(bc, m_A);
		break;
	}

	case 0x13: // LDW_IM 3by
	{
		m_B = m_memoryUnit.Read(m_PC++);
		m_C = m_memoryUnit.Read(m_PC++);
		break;
	}

	case 0x14: // INCW 1by
	{
		uint16_t BC = (m_B << 8) | m_C;
		++BC;
		m_B = (BC >> 8) & 0xFF;
		m_C = BC & 0xFF;
		break;
	}

	/* Logical operations */

	case 0x20: // ADD_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		uint16_t result = m_A + operand;
		m_CarryFlag = (result > 0xFF);

		m_A = static_cast<uint8_t>(result);
		UpdateFlags(m_A, oldA, operand, true);
		break;
	}

	case 0x21: // ADD_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		uint8_t reg = ReadRegister(selector);
		uint16_t result = m_A + reg;
		m_CarryFlag = (result > 0xFF);

		m_A = static_cast<uint8_t>(result);
		UpdateFlags(m_A, oldA, reg, true);
		break;
	}

	case 0x22: // ADC_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		uint16_t result = m_A + operand + (m_CarryFlag ? 1 : 0);
		m_CarryFlag = (result > 0xFF);

		m_A = static_cast<uint8_t>(result);
		UpdateFlags(m_A, oldA, operand, true);
		break;
	}

	case 0x23: // ADC_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		uint8_t reg = ReadRegister(selector);
		uint16_t result = m_A + reg + (m_CarryFlag ? 1 : 0);
		m_CarryFlag = (result > 0xFF);

		m_A = static_cast<uint8_t>(result);
		UpdateFlags(m_A, oldA, reg, true);
		break;
	}

	case 0x24: // SUB_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		m_CarryFlag = (operand > m_A);
		m_A -= operand;
		UpdateFlags(m_A, oldA, operand, false);
		break;
	}

	case 0x25: // SUB_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		uint8_t reg = ReadRegister(selector);
		m_CarryFlag = (reg > m_A);
		m_A -= reg;
		UpdateFlags(m_A, oldA, reg, false);
		break;
	}

	case 0x26: // SBC_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		bool oldCarry = m_CarryFlag;
		uint16_t result = m_A - operand - (oldCarry ? 1 : 0);

		m_CarryFlag = (m_A < operand + (oldCarry ? 1 : 0));
		m_A = static_cast<uint8_t>(result);
		UpdateFlags(m_A, oldA, operand, false);
		break;
	}

	case 0x27: // SBC_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t oldA = m_A;
		bool oldCarry = m_CarryFlag;
		uint8_t reg = ReadRegister(selector);
		uint16_t result = m_A - reg - (oldCarry ? 1 : 0);

		m_CarryFlag = (m_A < reg + (oldCarry ? 1 : 0));
		m_A = static_cast<uint8_t>(result);
		UpdateFlags(m_A, oldA, reg, false);
		break;
	}

	case 0x28: // INC 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t result = ReadRegister(selector) + 1;
		WriteRegister(selector, result);
		UpdateFlags(result);
		break;
	}

	case 0x29: // DEC 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t result = ReadRegister(selector) - 1;
		WriteRegister(selector, result);
		UpdateFlags(result);
		break;
	}

	case 0x2A: //MUL_IM 2by
	{
		m_A *= m_memoryUnit.Read(m_PC++);
		UpdateFlags(m_A);
		break;
	}

	case 0x2B: //MUL_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		m_A *= ReadRegister(selector);
		UpdateFlags(m_A);
		break;
	}

	/* Logical operations*/

	case 0x30: // AND_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		m_A = m_A & operand;
		UpdateFlags(m_A);
		m_CarryFlag = false;
		break;
	}

	case 0x31: // AND_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		m_A = m_A & reg;
		UpdateFlags(m_A);
		m_CarryFlag = false;
		break;
	}

	case 0x32: // OR_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		m_A = m_A | operand;
		UpdateFlags(m_A);
		m_CarryFlag = false;
		break;
	}

	case 0x33: // OR_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		m_A = m_A | reg;
		UpdateFlags(m_A);
		m_CarryFlag = false;
		break;
	}

	case 0x34: // XOR_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		m_A = m_A ^ operand;
		UpdateFlags(m_A);
		m_CarryFlag = false;
		break;
	}

	case 0x35: // XOR_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		m_A = m_A ^ reg;
		UpdateFlags(m_A);
		m_CarryFlag = false;
		break;
	}

	case 0x36: //NOT 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		reg = ~reg;
		WriteRegister(selector, reg);
		UpdateFlags(reg);
		m_CarryFlag = false;
		break;
	}

	case 0x37: //SHL 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		m_CarryFlag = (reg & 0x80) != 0;
		reg = reg << 1;
		WriteRegister(selector, reg);
		UpdateFlags(reg);
		break;
	}

	case 0x38: //SHR 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		m_CarryFlag = (reg & 0x01) != 0;
		reg = reg >> 1;
		WriteRegister(selector, reg);
		UpdateFlags(reg);
		break;
	}

	/* Compare instructions */

	case 0x40: // CMP_IM 2by
	{
		uint8_t operand = m_memoryUnit.Read(m_PC++);
		uint8_t result = m_A - operand;
		m_CarryFlag = (operand > m_A);
		UpdateFlags(result, m_A, operand, false);
		break;
	}

	case 0x41: // CMP_REG 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		uint8_t result = m_A - reg;
		m_CarryFlag = (reg > m_A);
		UpdateFlags(result, m_A, reg, false);
		break;
	}

	/* Jump/Branch instructions */

	case 0x50: // JMP 3by
	{
		uint8_t high = m_memoryUnit.Read(m_PC++);
		uint8_t low = m_memoryUnit.Read(m_PC++);
		uint16_t address = (high << 8) | low;
		m_PC = address;
		break;
	}

	case 0x51: // JZ 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (m_ZeroFlag) m_PC = address;
		break;
	}

	case 0x52: // JNZ 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (!m_ZeroFlag) m_PC = address;
		break;
	}

	case 0x53: // JC 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (m_CarryFlag) m_PC = address;
		break;
	}

	case 0x54: // JNC 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (!m_CarryFlag) m_PC = address;
		break;
	}

	case 0x55: // JN 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (m_NegativeFlag) m_PC = address;
		break;
	}

	case 0x56: // JNN 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (!m_NegativeFlag) m_PC = address;
		break;
	}

	case 0x57: // JO 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (m_OverflowFlag) m_PC = address;
		break;
	}

	case 0x58: // JNO 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;

		if (!m_OverflowFlag) m_PC = address;
		break;
	}

	/* Stack instructions */

	case 0x60: // PUSH 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		m_memoryUnit.Write(m_SP--, reg);
		break;
	}

	case 0x61: // POP 2by
	{
		uint8_t selector = m_memoryUnit.Read(m_PC++);
		uint8_t reg = ReadRegister(selector);
		reg = m_memoryUnit.Read(++m_SP);
		WriteRegister(selector, reg);
		break;
	}

	case 0x62: // CALL 3by
	{
		uint16_t address = ComputeAddress(m_PC);
		m_PC += 2;
		m_memoryUnit.Write(m_SP--, (m_PC >> 8) & 0xFF);
		m_memoryUnit.Write(m_SP--, m_PC & 0xFF);
		m_PC = address;
		break;
	}

	case 0x63: // RET 1by
	{
		uint8_t low = m_memoryUnit.Read(++m_SP);
		uint8_t high = m_memoryUnit.Read(++m_SP);
		m_PC = (high << 8) | low;
		break;
	}

	/* Misc instructions */

	case 0x00: // NOP 1by
	{
		break;
	}

	case 0x70: // MOV 3by
	{
		uint8_t destination = m_memoryUnit.Read(m_PC++);
		uint8_t source = m_memoryUnit.Read(m_PC++);
		WriteRegister(destination, ReadRegister(source));
		break;
	}

	case 0xF3: // EI 1by
	{
		m_interruptController.EnableInterrupts(true);
		break;
	}

	case 0xF8: // DI 1by
	{
		m_interruptController.EnableInterrupts(false);
		break;
	}

	case 0xF9: // RETI 1by
	{
		m_PC = m_memoryUnit[m_SP++];
		break;
	}

	case 0xFF: // HLT 1by
	{
		m_HaltFlag = true;
		break;
	}
	}
}

void CPU::Reset()
{
	m_A = 0u;
	m_B = 0u;
	m_C = 0u;
	m_D = 0u;
	m_PC = m_memoryUnit.GetStartAddress();
	m_SP = 0xFFFF;

	if (const auto result = m_memoryUnit.Peek(m_PC); result.has_value())
	{
		m_IR = result.value();
	}
	else
	{
		m_IR = 0x00;
	}

	m_ZeroFlag = false;
	m_CarryFlag = false;
	m_NegativeFlag = false;
	m_OverflowFlag = false;
	m_HaltFlag = false;
	m_interruptController.EnableInterrupts(false);
}

void CPU::HandleInterrupt()
{
	if (auto result = m_interruptController.PollPendingInterrupt(); result.has_value())
	{
		const uint8_t line = result.value();

		const uint8_t hiPC = (m_PC >> 8) & 0xFF;
		const uint8_t loPC = m_PC & 0xFF;

		// save PC address on the stack
		m_memoryUnit[m_SP--] = hiPC; 
		m_memoryUnit[m_SP--] = loPC;

		const uint16_t vectorAddress = line * 2;
		m_PC = ComputeAddress(vectorAddress);
	}
}

uint8_t CPU::ReadRegister(uint8_t selector) const noexcept
{
	switch (selector)
	{
	case 0x00: return m_A;
	case 0x01: return m_B;
	case 0x02: return m_C;
	case 0x03: return m_D;
	default: return 0x00;
	}
}

void CPU::WriteRegister(uint8_t selector, uint8_t value)
{
	switch (selector)
	{
	case 0x00: m_A = value; break;
	case 0x01: m_B = value; break;
	case 0x02: m_C = value; break;
	case 0x03: m_D = value; break;
	default: break;
	}
}

void CPU::UpdateFlags(uint8_t result)
{
	m_ZeroFlag = (result == 0);
	m_NegativeFlag = (result & 0x80) != 0;
}

void CPU::UpdateFlags(uint8_t result, uint8_t a, uint8_t operand, bool isAdd)
{
	m_ZeroFlag = (result == 0);
	m_NegativeFlag = (result & 0x80) != 0;

	if (isAdd)
	{
		m_OverflowFlag = (~(a ^ operand) & (a ^ result) & 0x80) != 0;
	}	
	else
	{
		m_OverflowFlag = ((a ^ operand) & (a ^ result) & 0x80) != 0;
	}
}

uint8_t CPU::GetA() const noexcept
{
	return m_A;
}

uint8_t CPU::GetB() const noexcept
{
	return m_B;
}

uint8_t CPU::GetC() const noexcept
{
	return m_C;
}

uint8_t CPU::GetD() const noexcept
{
	return m_D;
}

uint16_t CPU::GetPC() const noexcept
{
	return m_PC;
}

uint16_t CPU::GetSP() const noexcept
{
	return m_SP;
}

uint8_t CPU::GetIR() const noexcept
{
	return m_IR;
}

bool CPU::GetZeroFlag() const noexcept
{
	return m_ZeroFlag;
}

bool CPU::GetHaltFlag() const noexcept
{
	return m_HaltFlag;
}

bool CPU::GetCarryFlag() const noexcept
{
	return m_CarryFlag;
}

bool CPU::GetNegativeFlag() const noexcept
{
	return m_NegativeFlag;
}

bool CPU::GetOverflowFlag() const noexcept
{
	return m_OverflowFlag;
}

void CPU::SetProgramCounter(uint16_t address)
{
	m_PC = address;
}

bool CPU::IsReadingInstruction() const noexcept
{
	return m_memoryUnit.Read(m_PC) == 0x02;
}

bool CPU::IsWritingInstruction() const noexcept
{
	return m_memoryUnit.Read(m_PC) == 0x04;
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

uint16_t CPU::ComputeAddress(uint16_t address)
{
	uint8_t high = m_memoryUnit[address++];
	uint8_t low = m_memoryUnit[address++];
	return (high << 8) | low;
}
