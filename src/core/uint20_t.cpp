#include "Core/uint20_t.h"

constexpr uint20_t::uint20_t(uint32_t v) :
	m_value{v & MASK_20_BIT }
{
}

constexpr uint32_t uint20_t::value() const
{
	return m_value;
}

constexpr uint20_t uint20_t::operator+(uint32_t rhs) const
{
	return uint20_t(m_value + rhs);
}
