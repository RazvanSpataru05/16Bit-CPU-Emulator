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
	m_registers[0] = value;
}

void RegisterFile::Reset()
{
	std::fill(m_registers.begin(), m_registers.end(), 0x00);
}

uint8_t RegisterFile::A() const noexcept
{
	return m_registers[0];
}

uint8_t RegisterFile::B() const noexcept
{
	return m_registers[1];
}

uint8_t RegisterFile::C() const noexcept
{
	return m_registers[2];
}

uint8_t RegisterFile::D() const noexcept
{
	return m_registers[3];
}

uint8_t& RegisterFile::A()
{
	return m_registers[0];
}

uint8_t& RegisterFile::B()
{
	return m_registers[1];
}

uint8_t& RegisterFile::C()
{
	return m_registers[2];
}

uint8_t& RegisterFile::D()
{
	return m_registers[3];
}
