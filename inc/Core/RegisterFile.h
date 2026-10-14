#pragma once

#include <array>
#include <cstdint>

class RegisterFile
{
public:
	const size_t A_SELECTOR = 0;
	const size_t B_SELECTOR = 1;
	const size_t C_SELECTOR = 2;
	const size_t D_SELECTOR = 3;

public:
	RegisterFile() = default;

	uint8_t Read(uint8_t selector) const;
	void Write(uint8_t selector, uint8_t value);

	void UpdateA(uint8_t value);
	void Reset();

	// const getters
	uint8_t A() const noexcept;
	uint8_t B() const noexcept;
	uint8_t C() const noexcept;
	uint8_t D() const noexcept;

	// non-const reg getters
	uint8_t& A() noexcept;
	uint8_t& B() noexcept;
	uint8_t& C() noexcept;
	uint8_t& D() noexcept;

	uint16_t IX() const noexcept;
	uint16_t IY() const noexcept;

	uint16_t& IX() noexcept;
	uint16_t& IY() noexcept;


private:

	std::array<uint8_t, 4> m_registers{};

	// index registers
	uint16_t m_IX{};
	uint16_t m_IY{};

};