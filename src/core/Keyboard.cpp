#include "Core/Keyboard.h"

Keyboard::Keyboard(InterruptController& interruptController) :
	m_interruptController{ interruptController } 
{
}

void Keyboard::AddKey(uint16_t key)
{
	m_buffer.emplace(key);
}

std::optional<uint16_t> Keyboard::ConsumeKey()
{
	if (m_buffer.empty()) return std::nullopt;

	uint16_t key = m_buffer.front();
	m_buffer.pop();
	return key;
}