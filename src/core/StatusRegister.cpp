#include "StatusRegister.h"

StatusRegister::StatusRegister()
{
	Reset();
}

void StatusRegister::Reset()
{
	m_flags = Utils::Constants::FLAGS_DEFAULT_VALUE;
}

void StatusRegister::UpdateFrom(const Result& r) noexcept
{
	m_flags = static_cast<uint16_t>((m_flags & ~r.affected) | (r.flags & r.affected));

	std::bitset<16> flags(m_flags);
	std::cout << "Flags: " << flags << "\n";
}

uint16_t StatusRegister::GetFlags() const noexcept
{
	return m_flags;
}

uint8_t StatusRegister::Zero() const noexcept
{
	return (m_flags & Utils::Constants::ZF_MASK) != 0;
}

uint8_t StatusRegister::Sign() const noexcept
{
	return (m_flags & Utils::Constants::SF_MASK) != 0;
}

uint8_t StatusRegister::Carry() const noexcept
{
	return (m_flags & Utils::Constants::CF_MASK) != 0;
}

uint8_t StatusRegister::Overflow() const noexcept
{
	return (m_flags & Utils::Constants::OF_MASK) != 0;
}

uint8_t StatusRegister::Parity() const noexcept
{
	return (m_flags & Utils::Constants::PF_MASK) != 0;
}

uint8_t StatusRegister::Auxiliary() const noexcept
{
	return (m_flags & Utils::Constants::AF_MASK) != 0;
}

uint8_t StatusRegister::Trap() const noexcept
{
	return (m_flags & Utils::Constants::TF_MASK) != 0;
}

uint8_t StatusRegister::Interrupt() const noexcept
{
	return (m_flags & Utils::Constants::IF_MASK) != 0;
}

uint8_t StatusRegister::Directional() const noexcept
{
	return (m_flags & Utils::Constants::DF_MASK) != 0;
}

void StatusRegister::DisableInterrupts() noexcept
{
	m_flags &= ~(Utils::Constants::IF_MASK);
}
