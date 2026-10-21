#pragma once

#include "Core/Result.h"

class StatusRegister
{
public:
	StatusRegister() = default;

	void Reset();

	void UpdateFrom(const Result& result);

	bool Carry()		const noexcept;
	bool Overflow()		const noexcept;
	bool Zero()			const noexcept;
	bool Sign()			const noexcept;
	bool Parity()		const noexcept;
	bool Auxiliary()	const noexcept;

	bool& Carry();
	bool& Overflow();
	bool& Zero();
	bool& Sign();
	bool& Parity();
	bool& Auxiliary();

private:

	bool m_carryFlag{};
	bool m_overflowFlag{};
	bool m_zeroFlag{};
	bool m_signFlag{};
	bool m_parityFlag{};
	bool m_auxiliaryCarryFlag{};
};