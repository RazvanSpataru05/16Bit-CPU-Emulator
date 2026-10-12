#pragma once

#include "Core/Result.h"

class StatusRegister
{
public:
	StatusRegister() = default;

	void Reset();

	void UpdateFrom(const Result& result);

	bool Zero()			const noexcept;
	bool Negative()		const noexcept;
	bool Carry()		const noexcept;
	bool Overflow()		const noexcept;

	bool& Zero();
	bool& Negative();
	bool& Carry();
	bool& Overflow();

private:

	bool m_zeroFlag{};
	bool m_negativeFlag{};
	bool m_carryFlag{};
	bool m_overflowFlag{};
};