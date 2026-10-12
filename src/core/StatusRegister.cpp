#include "StatusRegister.h"

void StatusRegister::Reset()
{
	bool m_zeroFlag = false;
	bool m_negativeFlag = false;
	bool m_carryFlag = false;
	bool m_overflowFlag = false;
}

void StatusRegister::UpdateFrom(const Result& result)
{
	bool m_zeroFlag = result.zero;
	bool m_negativeFlag = result.negative;
	bool m_carryFlag = result.carry;
	bool m_overflowFlag = result.overflow;
}

bool StatusRegister::Zero() const noexcept
{
	return m_zeroFlag;
}

bool StatusRegister::Negative() const noexcept
{
	return m_negativeFlag;
}

bool StatusRegister::Carry() const noexcept
{
	return m_carryFlag;
}

bool StatusRegister::Overflow() const noexcept
{
	return m_overflowFlag;
}

bool& StatusRegister::Zero()
{
	return m_zeroFlag;
}

bool& StatusRegister::Negative()
{
	return m_negativeFlag;
}

bool& StatusRegister::Carry()
{
	return m_carryFlag;
}

bool& StatusRegister::Overflow()
{
	return m_overflowFlag;
}