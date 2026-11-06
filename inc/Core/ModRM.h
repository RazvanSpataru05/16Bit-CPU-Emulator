#pragma once

#include <cstdint>

struct ModRM
{
	uint8_t raw{};

	ModRM() = default;
	explicit ModRM(uint8_t r) :
		raw{ r } {}

	[[nodiscard]] uint8_t RM() const {  return  raw        & 0b00000111; }
	[[nodiscard]] uint8_t Reg() const { return (raw >> 3)  & 0b00000111; }
	[[nodiscard]] uint8_t Mod() const { return (raw >> 6)  & 0b00000011; }

};
