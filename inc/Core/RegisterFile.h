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

	const size_t ES_SELECTOR = 0;
	const size_t CS_SELECTOR = 1;
	const size_t SS_SELECTOR = 2;
	const size_t DS_SELECTOR = 3;

public:
	RegisterFile();

	uint16_t Read(uint16_t selector) const;
	void	 Write(uint16_t selector, uint16_t value);

	void UpdateA(uint16_t value);
	void Reset();
	
	uint8_t GetHi(size_t index) const;
	uint8_t GetLo(size_t index) const;
	uint16_t Get16(size_t index) const;

	void SetHi(size_t index, uint8_t value);
	void SetLo(size_t index, uint8_t value);
	void Set16(size_t index, uint16_t value);

	// const getters GP registers (lo part)
	uint8_t AL() const noexcept;
	uint8_t BL() const noexcept;
	uint8_t CL() const noexcept;
	uint8_t DL() const noexcept;

	// const getters GP registers (hi part)
	uint8_t AH() const noexcept;
	uint8_t BH() const noexcept;
	uint8_t CH() const noexcept;
	uint8_t DH() const noexcept;

	// const getters
	uint16_t AX() const noexcept;
	uint16_t BX() const noexcept;
	uint16_t CX() const noexcept;
	uint16_t DX() const noexcept;

	uint16_t SP() const noexcept;
	uint16_t BP() const noexcept;
	uint16_t SI() const noexcept;
	uint16_t DI() const noexcept;

	uint16_t ES() const noexcept;
	uint16_t CS() const noexcept;
	uint16_t SS() const noexcept;
	uint16_t DS() const noexcept;

	// non-const reg getters
	uint16_t& AX() noexcept;
	uint16_t& BX() noexcept;
	uint16_t& CX() noexcept;
	uint16_t& DX() noexcept;

	uint16_t& SP() noexcept;
	uint16_t& BP() noexcept;
	uint16_t& SI() noexcept;
	uint16_t& DI() noexcept;

	uint16_t& ES() noexcept;
	uint16_t& CS() noexcept;
	uint16_t& SS() noexcept;
	uint16_t& DS() noexcept;

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
	std::array<uint16_t, 8> m_generalPurproseReg{};
	std::array<uint16_t, 4> m_segmentReg{};
};