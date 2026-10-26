#pragma once

#include <cstdint>
#include <array>
#include <optional>

constexpr uint16_t INTERRUPT_LINE_COUNT{ 8u };
constexpr uint16_t KEYBOARD_INTERRUPT_LINE{ 0x00 };

class InterruptController
{

public:
	explicit InterruptController() = default;

	InterruptController(InterruptController&&) = default;
	InterruptController& operator=(InterruptController&&) = default;

	std::optional<uint16_t> PollPendingInterrupt();
	void RaiseInterrupt(uint16_t line);

private:
	InterruptController(const InterruptController&) = delete;
	InterruptController& operator=(const InterruptController&) = delete;

private:
	std::array<bool, INTERRUPT_LINE_COUNT> m_pending{};
};