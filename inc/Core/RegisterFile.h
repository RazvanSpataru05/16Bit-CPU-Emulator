#pragma once

#include <array>
#include <cstdint>

class RegisterFile
{
public:
	const size_t AX_SELECTOR = 0;
	const size_t CX_SELECTOR = 1;
	const size_t DX_SELECTOR = 2;
	const size_t BX_SELECTOR = 3;

	const size_t SP_SELECTOR = 4;
	const size_t BP_SELECTOR = 5;
	const size_t SI_SELECTOR = 6;
	const size_t DI_SELECTOR = 7;

public:
	RegisterFile() = default;

	uint16_t Read(uint16_t selector) const;
	void Write(uint16_t selector, uint16_t value);

	void UpdateA(uint16_t value);
	void Reset();
	
	uint16_t GetHi(size_t index) const;
	uint16_t GetLo(size_t index) const;

	// const getters
	uint16_t AX() const noexcept;
	uint16_t BX() const noexcept;
	uint16_t CX() const noexcept;
	uint16_t DX() const noexcept;

	uint16_t SP() const noexcept;
	uint16_t BP() const noexcept;
	uint16_t SI() const noexcept;
	uint16_t DI() const noexcept;

	// non-const reg getters
	uint16_t& AX() noexcept;
	uint16_t& BX() noexcept;
	uint16_t& CX() noexcept;
	uint16_t& DX() noexcept;

	uint16_t& SP() noexcept;
	uint16_t& BP() noexcept;
	uint16_t& SI() noexcept;
	uint16_t& DI() noexcept;


private:

	/*	0x00 - AX
		0x01 - CX
		0x02 - DX
		0x03 - BX

		0x04 - SP
		0x05 - BP
		0x06 - SI
		0x07 - DI
	*/
	std::array<uint16_t, 8> m_registers{};
};