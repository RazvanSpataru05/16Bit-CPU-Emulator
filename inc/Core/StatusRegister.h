#pragma once

#include "Core/Result.h"

#include "Utils/Constants.h"

#include <iostream>
#include <bitset>

class StatusRegister
{
public:

public:
	StatusRegister();

	void Reset();

	void UpdateFrom(const Result& r) noexcept;

	[[nodiscard]] uint8_t  GetLoFlags()		const noexcept;
	[[nodiscard]] uint8_t GetHiFlags()		const noexcept;
	[[nodiscard]] uint16_t GetFlags()		const noexcept;

	void SetLoFlags(uint8_t loFlags) noexcept;
	void SetHiFlags(uint8_t hiFlags) noexcept;
	void SetFlags(uint16_t flags) noexcept;

	[[nodiscard]]	uint8_t Carry()				const noexcept;
	[[nodiscard]]	uint8_t Overflow()			const noexcept;
	[[nodiscard]]	uint8_t Zero()				const noexcept;
	[[nodiscard]]	uint8_t Sign()				const noexcept;
	[[nodiscard]]	uint8_t Parity()			const noexcept;
	[[nodiscard]]	uint8_t Auxiliary()			const noexcept;

	[[nodiscard]]	uint8_t Trap()				const noexcept;
	[[nodiscard]]	uint8_t Interrupt()			const noexcept;
	[[nodiscard]]	uint8_t Directional()		const noexcept;

	void SetFlagOn(uint16_t mask) noexcept;
	void SetFlagOff(uint16_t mask) noexcept;

private:

	uint16_t m_flags{};
};