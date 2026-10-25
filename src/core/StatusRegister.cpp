#include "StatusRegister.h"

void StatusRegister::Reset()
{
	m_carryFlag =			false;
	m_overflowFlag =		false;
	m_zeroFlag =			false;
	m_signFlag =			false;
	m_parityFlag =			false;
	m_auxiliaryCarryFlag =	false;
}

void StatusRegister::UpdateFrom(const Result& result)
{
	if (result.affectsCarry)
	{
		m_carryFlag =		result.carry;
	}
	m_overflowFlag =		result.overflow;
	m_zeroFlag =			result.zero;
	m_signFlag =			result.sign;
	m_parityFlag =			result.parity;
	m_auxiliaryCarryFlag =	result.auxiliary;
}

bool StatusRegister::Zero() const noexcept
{
	return m_zeroFlag;
}

bool StatusRegister::Sign() const noexcept
{
	return m_signFlag;
}

bool StatusRegister::Carry() const noexcept
{
	return m_carryFlag;
}

bool StatusRegister::Overflow() const noexcept
{
	return m_overflowFlag;
}

bool StatusRegister::Parity() const noexcept
{
	return m_parityFlag;
}

bool StatusRegister::Auxiliary() const noexcept
{
	return m_auxiliaryCarryFlag;
}

bool& StatusRegister::Zero()
{
	return m_zeroFlag;
}

bool& StatusRegister::Sign()
{
	return m_signFlag;
}

bool& StatusRegister::Carry()
{
	return m_carryFlag;
}

bool& StatusRegister::Overflow()
{
	return m_overflowFlag;
}

bool& StatusRegister::Parity()
{
	return m_parityFlag;
}

bool& StatusRegister::Auxiliary()
{
	return m_auxiliaryCarryFlag;
}