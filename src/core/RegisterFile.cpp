#include "Core/RegisterFile.h"

uint16_t RegisterFile::Read(uint16_t selector) const
{
	return m_generalPurproseReg[selector];
}

void RegisterFile::Write(uint16_t selector, uint16_t value)
{
	m_generalPurproseReg[selector] = value;
}

void RegisterFile::UpdateA(uint16_t value)
{
	m_generalPurproseReg[AX_SELECTOR] += value;
}

void RegisterFile::Reset()
{
	std::fill(m_generalPurproseReg.begin(), m_generalPurproseReg.begin() + 3, 0x00);
	std::fill(m_segmentReg.begin(), m_segmentReg.end(), 0x00);

	m_generalPurproseReg[SP_SELECTOR] = 0xFFFF;

	m_segmentReg[CS_SELECTOR] = 0xFFFF;
}

uint16_t RegisterFile::GetHi(size_t index) const
{
	return (m_generalPurproseReg[index] >> 8);
}

uint16_t RegisterFile::GetLo(size_t index) const
{
	return (m_generalPurproseReg[index] & 0xFF);
}

uint16_t RegisterFile::AX() const noexcept
{
	return m_generalPurproseReg[AX_SELECTOR];
}

uint16_t RegisterFile::BX() const noexcept
{
	return m_generalPurproseReg[BX_SELECTOR];
}

uint16_t RegisterFile::CX() const noexcept
{
	return m_generalPurproseReg[CX_SELECTOR];
}

uint16_t RegisterFile::DX() const noexcept
{
	return m_generalPurproseReg[DX_SELECTOR];
}

uint16_t RegisterFile::SP() const noexcept
{
	return m_generalPurproseReg[SP_SELECTOR];
}

uint16_t RegisterFile::BP() const noexcept
{
	return m_generalPurproseReg[BP_SELECTOR];
}

uint16_t RegisterFile::SI() const noexcept
{
	return m_generalPurproseReg[SI_SELECTOR];
}

uint16_t RegisterFile::DI() const noexcept
{
	return m_generalPurproseReg[DI_SELECTOR];
}

uint16_t RegisterFile::ES() const noexcept
{
	return m_segmentReg[ES_SELECTOR];
}

uint16_t RegisterFile::CS() const noexcept
{
	return m_segmentReg[CS_SELECTOR];
}

uint16_t RegisterFile::SS() const noexcept
{
	return m_segmentReg[SS_SELECTOR];
}

uint16_t RegisterFile::DS() const noexcept
{
	return m_segmentReg[DS_SELECTOR];
}

uint16_t& RegisterFile::AX() noexcept
{
	return m_generalPurproseReg[AX_SELECTOR];
}

uint16_t& RegisterFile::BX() noexcept
{
	return m_generalPurproseReg[BX_SELECTOR];
}

uint16_t& RegisterFile::CX() noexcept
{
	return m_generalPurproseReg[CX_SELECTOR];
}

uint16_t& RegisterFile::DX() noexcept
{
	return m_generalPurproseReg[DX_SELECTOR];
}

uint16_t& RegisterFile::SP() noexcept
{
	return m_generalPurproseReg[SP_SELECTOR];
}

uint16_t& RegisterFile::BP() noexcept
{
	return m_generalPurproseReg[BP_SELECTOR];
}

uint16_t& RegisterFile::SI() noexcept
{
	return m_generalPurproseReg[SI_SELECTOR];
}

uint16_t& RegisterFile::DI() noexcept
{
	return m_generalPurproseReg[DI_SELECTOR];
}

uint16_t& RegisterFile::ES() noexcept
{
	return m_segmentReg[ES_SELECTOR];
}

uint16_t& RegisterFile::CS() noexcept
{
	return m_segmentReg[CS_SELECTOR];
}

uint16_t& RegisterFile::SS() noexcept
{
	return m_segmentReg[SS_SELECTOR];
}

uint16_t& RegisterFile::DS() noexcept
{
	return m_segmentReg[DS_SELECTOR];
}
