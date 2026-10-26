#include "Core/RegisterFile.h"

RegisterFile::RegisterFile()
{
	Reset();
}

uint16_t RegisterFile::Read(uint16_t selector) const
{
	return m_generalPurproseReg[selector];
}

void RegisterFile::Write(uint16_t selector, uint16_t value)
{
	m_generalPurproseReg[selector] = value;
}

void RegisterFile::UpdateAX(uint16_t value)
{
	m_generalPurproseReg[AX_SELECTOR] += value;
}

void RegisterFile::UpdateCS(uint16_t value)
{
	m_segmentReg[CS_SELECTOR] += value;
}

void RegisterFile::Reset()
{
	std::fill(m_generalPurproseReg.begin(), m_generalPurproseReg.end(), 0x0000);
	std::fill(m_segmentReg.begin(), m_segmentReg.end(), 0x0000);

	m_generalPurproseReg[SP_SELECTOR] = 0x0100;
}

uint8_t RegisterFile::Get8(Reg8 reg) const noexcept
{
	const size_t idx = static_cast<size_t>(reg);
	if (idx < 4) // Get lo GP reg
	{
		return static_cast<uint8_t>(m_generalPurproseReg[idx] & 0x00FF);
	}
	else // Get hi GP reg
	{
		return static_cast<uint8_t>((m_generalPurproseReg[idx - 4] & 0xFF00) >> 8);
	}
}

uint16_t RegisterFile::Get16(Reg16 reg) const noexcept
{
	return m_generalPurproseReg[static_cast<size_t>(reg)];
}

uint16_t& RegisterFile::Get16(Reg16 reg) noexcept
{
	return m_generalPurproseReg[static_cast<size_t>(reg)];
}

void RegisterFile::SetGP8(Reg8 reg, uint8_t value)
{
	size_t idx = static_cast<size_t>(reg);
	if (idx < 4) // lo part of GP reg (AL, CL, DL, BL)
	{
		uint16_t newValue = (m_generalPurproseReg[idx] & 0xFF00) | value;
		m_generalPurproseReg[idx] = newValue;
	}
	else // hi part of GP reg (AH, CH, DH, BH)
	{
		uint16_t newValue = static_cast<uint16_t>(value << 8) | static_cast<uint16_t>(m_generalPurproseReg[idx - 4] & 0x00FF);
		m_generalPurproseReg[idx - 4] = newValue;
	}
}

void RegisterFile::SetGP16(Reg16 selector, uint16_t value)
{
	m_generalPurproseReg[static_cast<size_t>(selector)] = value;
}

void RegisterFile::SetSEG(SegmentReg reg, uint16_t value)
{
	m_segmentReg[static_cast<size_t>(reg)] = value;
}

void RegisterFile::IncrementSP()
{
	++m_segmentReg[SP_SELECTOR];
}

void RegisterFile::DecrementSP()
{
	--m_segmentReg[SP_SELECTOR];
}

uint8_t RegisterFile::AL() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[AX_SELECTOR] & 0xFF);
}

uint8_t RegisterFile::BL() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[BX_SELECTOR] & 0xFF);
}

uint8_t RegisterFile::CL() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[CX_SELECTOR] & 0xFF);
}

uint8_t RegisterFile::DL() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[DX_SELECTOR] & 0xFF);
}

uint8_t RegisterFile::AH() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[AX_SELECTOR] >> 8);
}

uint8_t RegisterFile::BH() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[BX_SELECTOR] >> 8);
}

uint8_t RegisterFile::CH() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[CX_SELECTOR] >> 8);
}

uint8_t RegisterFile::DH() const noexcept
{
	return static_cast<uint8_t>(m_generalPurproseReg[DX_SELECTOR] >> 8);
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
