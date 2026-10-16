#include "Core/RegisterFile.h"

uint16_t RegisterFile::Read(uint16_t selector) const
{
	return m_registers[selector];
}

void RegisterFile::Write(uint16_t selector, uint16_t value)
{
	m_registers[selector] = value;
}

void RegisterFile::UpdateA(uint16_t value)
{
	m_registers[AX_SELECTOR] += value;
}

void RegisterFile::Reset()
{
	std::fill(m_registers.begin(), m_registers.begin() + 3, 0x00);
	m_registers[SP_SELECTOR] = 0xFFFF;
}

uint16_t RegisterFile::GetHi(size_t index) const
{
	return (m_registers[index] >> 8);
}

uint16_t RegisterFile::GetLo(size_t index) const
{
	return (m_registers[index] & 0xFF);
}

uint16_t RegisterFile::AX() const noexcept
{
	return m_registers[AX_SELECTOR];
}

uint16_t RegisterFile::BX() const noexcept
{
	return m_registers[BX_SELECTOR];
}

uint16_t RegisterFile::CX() const noexcept
{
	return m_registers[CX_SELECTOR];
}

uint16_t RegisterFile::DX() const noexcept
{
	return m_registers[DX_SELECTOR];
}

uint16_t RegisterFile::SP() const noexcept
{
	return m_registers[SP_SELECTOR];
}

uint16_t RegisterFile::BP() const noexcept
{
	return m_registers[BP_SELECTOR];
}

uint16_t RegisterFile::SI() const noexcept
{
	return m_registers[SI_SELECTOR];
}

uint16_t RegisterFile::DI() const noexcept
{
	return m_registers[DI_SELECTOR];
}

uint16_t& RegisterFile::AX() noexcept
{
	return m_registers[AX_SELECTOR];
}

uint16_t& RegisterFile::BX() noexcept
{
	return m_registers[BX_SELECTOR];
}

uint16_t& RegisterFile::CX() noexcept
{
	return m_registers[CX_SELECTOR];
}

uint16_t& RegisterFile::DX() noexcept
{
	return m_registers[DX_SELECTOR];
}

uint16_t& RegisterFile::SP() noexcept
{
	return m_registers[SP_SELECTOR];
}

uint16_t& RegisterFile::BP() noexcept
{
	return m_registers[BP_SELECTOR];
}

uint16_t& RegisterFile::SI() noexcept
{
	return m_registers[SI_SELECTOR];
}

uint16_t& RegisterFile::DI() noexcept
{
	return m_registers[DI_SELECTOR];
}
