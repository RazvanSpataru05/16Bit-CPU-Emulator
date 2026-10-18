#include "Core/uint20_t.h"

uint20_t& uint20_t::operator++()
{
	(++m_value & Utils::MASK_20_BIT);
	return *this;
}

uint20_t& uint20_t::operator++(int)
{	
	(++m_value & Utils::MASK_20_BIT);
	return *this;
}

uint20_t uint20_t::operator+(uint32_t rhs)
{
	return uint20_t(m_value + rhs);
}
