#pragma once

#include "Utils/Constants.h"

#include <assert.h>
#include <array>

class IOPortSpace
{
public:

	explicit IOPortSpace();

	[[nodiscard]] uint8_t ReadPort8(size_t port)		const noexcept;
	[[nodiscard]] uint16_t ReadPort16(size_t port)		const noexcept;

	void WritePort8(size_t port, uint8_t value)			noexcept;
	void WritePort16(size_t port, uint16_t value)		noexcept;
	
	[[nodiscard]] const uint8_t& operator[](size_t port) const noexcept;
	[[nodiscard]] uint8_t& operator[](size_t port)		 noexcept;


private:
	IOPortSpace(const IOPortSpace&) = delete;
	IOPortSpace& operator=(const IOPortSpace&) = delete;

	IOPortSpace(IOPortSpace&&) = delete;
	IOPortSpace& operator=(IOPortSpace&&) = delete;

private:
	std::array<uint8_t, Utils::Constants::PORT_COUNT> m_ports{};
};