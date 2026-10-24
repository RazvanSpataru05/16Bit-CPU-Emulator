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
	++m_IP;
	assert(m_memoryUnit[PhysAddr()].has_value());
	return m_memoryUnit[PhysAddr()].value();
}

uint16_t CPU::Fetch16()
{
	++m_IP;
	assert(m_memoryUnit[PhysAddr()].has_value());
	uint8_t lo = m_memoryUnit[PhysAddr()].value();
	
	++m_IP;
	assert(m_memoryUnit[PhysAddr()].has_value());
	uint8_t hi = m_memoryUnit[PhysAddr()].value();

	return (static_cast<uint16_t>(hi) << 8) | static_cast<uint16_t>(lo);
}

void CPU::Step()
{
	if (m_haltFlag || m_memoryUnit.IsMemoryEmpty()) return;

	if ((m_IR & 0xC0) == 0x00 && (m_IR & 0b111) < 6) ExecALU();
	else if ((m_IR & 0xF0) == 0xB0)					 ExecMOVImm();
	else switch (m_IR)
	{
	case 0x90: break; // NOP 1by
	case 0xF4: m_haltFlag = true; break; // HLT 1by
	default:
		Logger::AddInfoMessage(std::format("Unimplemented opcode: {:02X}", m_IR));
		break;
	}
	UpdateIR();
}

void CPU::ExecALU()
{
	const uint8_t operation = (m_IR >> 3) & 0b111;
	const uint8_t form = m_IR & 0b111;
	if (form < 4) return; // TODO: Implement Mod R/M

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
	++m_IP;
}

void CPU::ExecMOVImm()
{
	const bool word = (m_IR >> 3) & 1; // word == 1 -> full reg / word == 0 -> Hi / Lo
	const uint8_t selector = m_IR & 0b111;
	std::cout << std::hex << static_cast<int>(selector);

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

	assert(m_memoryUnit[PhysAddr()].has_value());
	m_IR = m_memoryUnit[PhysAddr()].value();
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
