#pragma once

#include "Core/InterruptController.h"

#include <optional>
#include <cstdint>
#include <queue>

class Keyboard
{
public:
	explicit Keyboard(InterruptController& interruptController);

	void AddKey(uint16_t key);
	std::optional<uint16_t> ConsumeKey();

private:
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;

	Keyboard(Keyboard&&) = delete;
	Keyboard& operator=(Keyboard&&) = delete;

private:
	InterruptController& m_interruptController;
	std::queue<uint16_t> m_buffer;
};