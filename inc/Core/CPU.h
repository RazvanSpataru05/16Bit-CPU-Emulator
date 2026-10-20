#pragma once

#include "Core/ALU.h"
#include "Core/InterruptController.h"
#include "Core/RegisterFile.h"
#include "Core/StatusRegister.h"
#include "Core/MemoryUnit.h"
#include "Core/ModRM.h"

#include <string>
#include <assert.h>

class CPU
{
public:
	const size_t MEMORY_UNIT_SIZE{ 65536 };

public:
	explicit CPU(InterruptController& interruptContrroller);

	CPU(CPU&&) = default;
	CPU& operator=(CPU&&) = default;

	uint20_t	PhysAddr()		const noexcept;

	uint8_t		Fetch8();
	uint16_t	Fetch16();

	void Fetch();
	void Step();
	void Reset();

	void HandleInterrupt();

	uint16_t ComputeAddress(uint16_t address);

	//Registry getters
	uint16_t		GetIP()			const noexcept;
	uint16_t		GetIR()			const noexcept;

	bool			GetHaltFlag()	const noexcept;

	// PC setter
	void SetProgramCounter(uint16_t address);
	void SetInstructionRegister();

	bool IsReadingInstruction() const noexcept;
	bool IsWritingInstruction() const noexcept;

	// Memory Unit / Interrupt Controller getter (const & non-const)
	[[nodiscard]] const MemoryUnit& GetMemoryUnit()						const noexcept;
	[[nodiscard]] MemoryUnit& GetMemoryUnit()							noexcept;
	[[nodiscard]] const InterruptController& GetInterruptController()	const noexcept;
	[[nodiscard]] InterruptController& GetInterruptController()			noexcept;

	[[nodiscard]] const RegisterFile& GetRegisterFile()					const noexcept;
	[[nodiscard]] const StatusRegister& GetStatusRegister()				const noexcept;

private:

private:
	CPU(const CPU&) = delete;
	CPU& operator=(const CPU&) = delete;

private:

	MemoryUnit m_memoryUnit;
	InterruptController& m_interruptController;

	RegisterFile m_registerFile;
	StatusRegister m_statusRegister;

	uint16_t m_IP{};		// Instruction Pointer Registry (offset, 16 bits) 
	uint16_t m_IR{};		// Instruction Registry

	bool m_haltFlag{ false };
};

