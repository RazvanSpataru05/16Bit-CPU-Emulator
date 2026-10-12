#pragma once

#include <array>
#include <cstdint>

class RegisterFile
{
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
	uint8_t& A();
	uint8_t& B();
	uint8_t& C();
	uint8_t& D();

private:

	std::array<uint8_t, 4> m_registers{};
};