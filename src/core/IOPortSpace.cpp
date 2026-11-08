#include "Core/IOPortSpace.h"

IOPortSpace::IOPortSpace()
{
	std::fill(m_ports.begin(), m_ports.end(), 0xFF);
}

uint8_t IOPortSpace::ReadPort8(size_t port) const noexcept
{
	return m_ports[port];
}

uint16_t IOPortSpace::ReadPort16(size_t port) const noexcept
{
	const uint8_t lo = ReadPort8(port);
	const uint8_t hi = ReadPort8(port + 1);
	return static_cast<uint16_t>((hi << 8) | lo);
}

void IOPortSpace::WritePort8(size_t port, uint8_t value) noexcept
{
	m_ports[port] = value;
}

void IOPortSpace::WritePort16(size_t port, uint16_t value) noexcept
{
	WritePort8(port, value & 0xFF);
	WritePort8(port + 1, static_cast<uint8_t>(value >> 8));
}

uint8_t& IOPortSpace::operator[](size_t port) noexcept
{
	assert(port < m_ports.size());

	return m_ports[port];
}

const uint8_t& IOPortSpace::operator[](size_t port) const noexcept
{
	assert(port < m_ports.size());

	return m_ports[port];
}


