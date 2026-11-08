#pragma once

#include "Core/ALU.h"
#include "Core/InterruptController.h"
#include "Core/RegisterFile.h"
#include "Core/StatusRegister.h"
#include "Core/SegmentOverride.h"
#include "Core/MemoryUnit.h"
#include "Core/ModRM.h"
#include "Core/RMOperand.h"

#include <string>
#include <assert.h>

class CPU
{
public:
	explicit CPU(InterruptController& interruptContrroller);

	[[nodiscard]] uint20_t	PhysAddr(uint16_t segment, uint16_t offset)		const noexcept;

	[[nodiscard]] uint8_t		Fetch8();
	[[nodiscard]] uint16_t		Fetch16();

	void Step();
	void Reset();
	
	[[nodiscard]] SegmentOverride GetSegmentOverride() const noexcept;

	//Registry getters
	[[nodiscard]] uint16_t		GetIP()			const noexcept;
	[[nodiscard]] uint8_t		GetIR()			const noexcept;

	[[nodiscard]] bool			GetHaltFlag()	const noexcept;

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
	[[nodiscard]] uint16_t ReadReg(uint8_t selector, bool word) const;
	void WriteReg(uint8_t selector, uint16_t value, bool word, bool generalReg);

	[[nodiscard]] uint16_t ReadStackWord(uint16_t stackPointer);

	[[nodiscard]] uint8_t  ReadMem8(uint16_t segment, uint16_t offset);
	[[nodiscard]] uint16_t ReadMem16(uint16_t segment, uint16_t offset);

	void WriteMem8(uint16_t segment, uint16_t offset, uint8_t value);
	void WriteMem16(uint16_t segment, uint16_t offset, uint16_t value);

	[[nodiscard]] uint16_t ReadRM(const RMOperand& operand, bool word);
	void     WriteRM(const RMOperand& operand, uint16_t value, bool word, bool generalReg);

	void ShortJump(int8_t offset);

	/* Stack Helpers */
	void Push16(uint16_t value, bool word);
	[[nodiscard]] uint16_t Pop16();

	[[nodiscard]] bool							  IsPrefixOverride(uint8_t value)		const noexcept;
	[[nodiscard]] SegmentOverride				  DecodeSegmentOverride()				noexcept;
	[[nodiscard]] std::optional<uint16_t>		  ResolveSegment()						noexcept;

	template<typename RegType>
	void StackOperation(RegType regType, bool isPush);

	void ExecALU();
	void ExecPushPopGP();
	void ExecPushPopSeg();
	void ExecPushPopFlags();
	void ExecConditionedJump();
	void ExecXCHG();
	void ExecXCHGModRM();
	void ExecStoreRegIntoFlags();
	void ExecConvert();
	void ExecMOVImm();
	void ExecRet();
	void ExecRetFar();
	void ExecMOVAccumulator();
	void ExecMOVModRM();
	void ExecINCDEC();
	void ExecLoop();
	void ExecClearStoreFlags();
	void ExecGRP1();
	void ExecGRP2();
	void ExecGRP4_GRP5();
	void ExecTEST();
	void ExecTESTModRM();
	void ExecMOVS();
	void ExecCMPS();
	void ExecSTOS();
	void ExecLODS();
	void ExecSCAS();
	void ExecDoubleWord();
	void ExecMOVRMImm();
	void ExecInterrupt(uint8_t interrupt);
	void ExecESC();

	void DecimalAdjust(bool isSubtract);

	[[nodiscard]] ModRM DecodeModRM();
	[[nodiscard]] uint16_t DecodeEffectiveAddress(uint8_t rm, uint8_t mod);

	[[nodiscard]] RMOperand ResolveRM(ModRM modRM);

	void UpdateDirection(bool directionalFlag, bool updateSource, bool updateDest, uint8_t value) noexcept;

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

	SegmentOverride m_segmentOverride{};

	bool m_haltFlag{ false };
	bool m_TESTPin{ false };
};

template<typename RegType>
void CPU::StackOperation(RegType regType, bool isPush)
{
	if (isPush)
	{
		const uint16_t value = m_registerFile.Get16(regType);
		Push16(value, true);
	}
	else
	{
		const uint16_t sp = static_cast<uint16_t>(m_registerFile.SP());
		const uint16_t word = Pop16();
		m_registerFile.Set16(regType, word);
	}
	++m_IP;
}