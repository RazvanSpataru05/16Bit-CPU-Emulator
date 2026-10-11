#pragma once

#include "Core/InterruptController.h"
#include "Core/MemoryUnit.h"

#include <string>

class CPU
{
public:
	const size_t MEMORY_UNIT_SIZE{ 65536 };

public:
	explicit CPU(InterruptController& interruptContrroller);

	CPU(CPU&&) = default;
	CPU& operator=(CPU&&) = default;

	void Fetch();
	void Step();
	void Reset();

	void HandleInterrupt();

	uint16_t ComputeAddress(uint16_t address);

	//Registry getters
	[[nodiscard]] uint8_t		GetA()		const noexcept;
	[[nodiscard]] uint8_t		GetB()		const noexcept;
	[[nodiscard]] uint8_t		GetC()		const noexcept;
	[[nodiscard]] uint8_t		GetD()		const noexcept;

	[[nodiscard]] uint16_t		GetPC()		const noexcept;
	[[nodiscard]] uint16_t		GetSP()		const noexcept;
	[[nodiscard]] uint8_t		GetIR()		const noexcept;

	//Flag getters
	[[nodiscard]] bool GetZeroFlag()		const noexcept;
	[[nodiscard]] bool GetHaltFlag()		const noexcept;
	[[nodiscard]] bool GetCarryFlag()		const noexcept;
	[[nodiscard]] bool GetNegativeFlag()	const noexcept;
	[[nodiscard]] bool GetOverflowFlag()	const noexcept;

	// PC setter
	void SetProgramCounter(uint16_t address);

	bool IsReadingInstruction() const noexcept;
	bool IsWritingInstruction() const noexcept;

	// Memory Unit / Interrupt Controller getter (const & non-const)
	[[nodiscard]] const MemoryUnit& GetMemoryUnit()						const noexcept;
	[[nodiscard]] MemoryUnit& GetMemoryUnit()							noexcept;
	[[nodiscard]] const InterruptController& GetInterruptController()	const noexcept;
	[[nodiscard]] InterruptController& GetInterruptController()			noexcept;

private:
	[[nodiscard]] uint8_t	ReadRegister(uint8_t selector) const noexcept;
	void					WriteRegister(uint8_t selector, uint8_t value);

	void UpdateFlags(uint8_t result);
	void UpdateFlags(uint8_t result, uint8_t a, uint8_t opearand, bool isAdd);

private:
	CPU(const CPU&) = delete;
	CPU& operator=(const CPU&) = delete;

private:

	MemoryUnit m_memoryUnit;
	InterruptController& m_interruptController;

	// General Registry
	uint8_t m_A{};
	uint8_t m_B{};
	uint8_t m_C{};
	uint8_t m_D{};

	uint16_t m_SP{};		// Stack Pointer Registry
	uint16_t m_PC{};		// Program Counter Registry
	uint8_t m_IR{};			// Instruction Registry

	bool m_ZeroFlag{ false };
	bool m_CarryFlag{ false };
	bool m_NegativeFlag{ false };
	bool m_OverflowFlag{ false };
	bool m_HaltFlag{ false };
};

