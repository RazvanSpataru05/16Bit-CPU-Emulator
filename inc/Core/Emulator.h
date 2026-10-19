#pragma once

#include "Core/Toolchain.h"
#include "Core/CPU.h"
#include "Core/Keyboard.h"
#include "Core/Terminal.h"


class Emulator
{
public:
	Emulator();

	[[nodiscard]] const CPU& GetCPU()								const noexcept;
	[[nodiscard]] CPU& GetCPU()						 				noexcept;

	[[nodiscard]]  Toolchain& GetToolchain()						noexcept;
	[[nodiscard]]  InterruptController& GetInterruptController()	noexcept;
	[[nodiscard]]  Keyboard& GetKeyboard()							noexcept;
	[[nodiscard]]  Terminal& GetTerminal()							noexcept;


private:
	Emulator(const Emulator&) = delete;
	Emulator& operator=(const Emulator&) = delete;

	Emulator(Emulator&&) = delete;
	Emulator& operator=(Emulator&&) = delete;

private:
	Toolchain m_toolchain;
	InterruptController m_interruptController;
	CPU m_cpu;
	Keyboard m_keyboard;
	Terminal m_terminal;
};