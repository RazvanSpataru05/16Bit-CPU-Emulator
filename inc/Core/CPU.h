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
	explicit CPU(InterruptController& interruptContrroller);

	uint20_t	PhysAddr(uint16_t segment, uint16_t offset)		const noexcept;

	uint8_t		Fetch8();
	uint16_t	Fetch16();

	void Step();
	void Reset();

	void HandleInterrupt();

	//Registry getters
	uint16_t		GetIP()			const noexcept;
	uint8_t			GetIR()			const noexcept;

	bool			GetHaltFlag()	const noexcept;

	void SetInstructionPointer(uint16_t address);
	void SetInstructionRegister();

	void UpdateIR();

	bool IsReadingInstruction() const noexcept;
	bool IsWritingInstruction() const noexcept;

	// Memory Unit / Interrupt Controller getter (const & non-const)
	[[nodiscard]] const MemoryUnit& GetMemoryUnit()						const noexcept;
	[[nodiscard]] MemoryUnit& GetMemoryUnit()							noexcept;
	[[nodiscard]] const InterruptController& GetInterruptController()	const noexcept;
	[[nodiscard]] InterruptController& GetInterruptController()			noexcept;

	[[nodiscard]] const RegisterFile& GetRegisterFile()					const noexcept;
	[[nodiscard]] RegisterFile& GetRegisterFile()						noexcept;
	[[nodiscard]] const StatusRegister& GetStatusRegister()				const noexcept;
	[[nodiscard]] StatusRegister& GetStatusRegister()					noexcept;

private:
	uint16_t ReadReg(uint8_t selector, bool word) const;
	void WriteReg(uint8_t selector, uint16_t value, bool word);

	uint16_t ReadStackWord(uint16_t stackPointer);

	void ShortJump(int8_t offset);

	/* Stack Helpers */
	void Push16(uint16_t value);
	uint16_t Pop16();

	template<typename RegType>
	void StackOperation(RegType selector, bool isPush);

	void ExecALU();
	void ExecPushPopGP();
	void ExecPushPopSeg();
	void ExecPushPopFlags();
	void ExecConditionedJump();
	void ExecExchange();
	void ExecStoreRegIntoFlags();
	void ExecConvert();
	void ExecMOVImm();
	void ExecRet();
	void ExecRetFar();
	void ExecMOVModRM();
	void ExecINCDEC();
	void ExecLoop();

	ModRM DecodeModRM();

private:
	CPU(const CPU&) = delete;
	CPU& operator=(const CPU&) = delete;

	CPU(CPU&&) = delete;
	CPU& operator=(CPU&&) = delete;

private:

	MemoryUnit m_memoryUnit;
	InterruptController& m_interruptController;

	RegisterFile m_registerFile;
	StatusRegister m_statusRegister;

	uint16_t	m_IP{};		// Instruction Pointer Registry (offset, 16 bits) 
	uint8_t		m_IR{};		// Instruction Registry

	bool m_haltFlag{ false };
	bool m_TESTPin{ false };
};

