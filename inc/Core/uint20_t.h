#pragma once

#include "Utils/Constants.h"

#include <cstdint>
#include <functional>

class uint20_t
{
public:
	constexpr explicit uint20_t(uint32_t v = 0) : m_value{ v & Utils::MASK_20_BIT } {};
	constexpr uint20_t(uint16_t v) : m_value{ v } {};

	uint20_t(const uint20_t&) = default;
	uint20_t& operator=(const uint20_t&) = default;

	uint20_t(uint20_t&&) = default;
	uint20_t& operator=(uint20_t&&) = default;

	constexpr uint32_t index() const { return m_value; };

	uint20_t& operator++();
	uint20_t& operator++(int);

	constexpr uint20_t& operator=(uint32_t rhs) { m_value = rhs & Utils::MASK_20_BIT; return *this; };
	constexpr bool operator==(uint20_t rhs) const { return m_value == rhs.index(); };
	uint20_t operator+(uint32_t rhs);

private:
	uint32_t m_value;
};

namespace std
{
	template<>
	struct hash<uint20_t>
	{
		std::size_t operator()(uint20_t value) const
		{
			std::size_t h1 = std::hash<uint32_t>{}(value.index());

			return h1;
		}
	};
}