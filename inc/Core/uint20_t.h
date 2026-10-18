#pragma once

#include <cstdint>

constexpr uint32_t MASK_20_BIT{ 0xFFFFF };

class uint20_t
{
public:
	constexpr explicit uint20_t(uint32_t v = 0);
	constexpr uint32_t value() const;
	constexpr uint20_t operator+(uint32_t rhs) const;

private:
	uint32_t m_value;
};