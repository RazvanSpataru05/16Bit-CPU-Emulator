#include "Core/Emulator.h"

Emulator::Emulator() :
	m_toolchain{ Toolchain() },
	m_interruptController{ InterruptController() },
	m_cpu{ CPU(m_interruptController) },
	m_keyboard{ Keyboard(m_interruptController) },
	m_terminal{ Terminal() }
{

}

const CPU& Emulator::GetCPU() const noexcept
{
	return m_cpu;
}

CPU& Emulator::GetCPU() noexcept
{
	return m_cpu;
}

Toolchain& Emulator::GetToolchain() noexcept
{
	return m_toolchain;
}

InterruptController& Emulator::GetInterruptController() noexcept
{
	return m_interruptController;
}

Keyboard& Emulator::GetKeyboard() noexcept
{
	return m_keyboard;
}

Terminal& Emulator::GetTerminal() noexcept
{
	return m_terminal;
}

