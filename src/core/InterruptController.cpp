#include "Core/InterruptController.h"

void InterruptController::RaiseInterrupt(uint16_t line)
{
	m_pending[line] = true;
}

std::optional<uint16_t> InterruptController::PollPendingInterrupt()
{
	for (uint16_t line = 0; line < INTERRUPT_LINE_COUNT; ++line)
	{
		if (m_pending[line])
		{
			m_pending[line] = false;
			return line;
		}
	}
	return std::nullopt;
}

bool InterruptController::GetInterruptFlag() const noexcept
{
	return m_interruptFlag;
}

void InterruptController::EnableInterrupts(bool enabled) noexcept
{
	m_interruptFlag = enabled;
}
