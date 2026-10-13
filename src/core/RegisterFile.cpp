#include "Core/RegisterFile.h"

uint8_t RegisterFile::Read(uint8_t selector) const
{
	return m_registers[selector];
}

void RegisterFile::Write(uint8_t selector, uint8_t value)
{
	m_registers[selector] = value;
}

void RegisterFile::UpdateA(uint8_t value)
{
	m_registers[A_SELECTOR] += value;
}

void RegisterFile::Reset()
{
	std::fill(m_registers.begin(), m_registers.end(), 0x00);
	m_IX = 0x0000;
	m_IY = 0x0000;
}

uint8_t RegisterFile::A() const noexcept
{
	return m_registers[A_SELECTOR];
}

uint8_t RegisterFile::B() const noexcept
{
	return m_registers[B_SELECTOR];
}

uint8_t RegisterFile::C() const noexcept
{
	return m_registers[C_SELECTOR];
}

uint8_t RegisterFile::D() const noexcept
{
	return m_registers[D_SELECTOR];
}

uint8_t& RegisterFile::A() noexcept
{
	return m_registers[A_SELECTOR];
}

uint8_t& RegisterFile::B() noexcept
{
	return m_registers[B_SELECTOR];
}

uint8_t& RegisterFile::C() noexcept
{
	return m_registers[C_SELECTOR];
}

uint8_t& RegisterFile::D() noexcept
{
	return m_registers[D_SELECTOR];
}

uint16_t RegisterFile::IX() const noexcept
{
	return m_IX;
}

uint16_t RegisterFile::IY() const noexcept
{
	return m_IY;
}

uint16_t& RegisterFile::IX() noexcept
{
	return m_IX;
}

uint16_t& RegisterFile::IY() noexcept
{
	return m_IY;
}
