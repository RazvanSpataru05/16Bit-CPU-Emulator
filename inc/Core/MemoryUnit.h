#pragma once

#include "IO/DataLoader.h"

#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <span>

const size_t MEMORY_SIZE{ 65536 };

class MemoryUnit
{
public:
	explicit MemoryUnit();

	MemoryUnit(MemoryUnit&&) = default;
	MemoryUnit& operator=(MemoryUnit&&) = default;

	const std::optional<uint8_t>& Peek(uint16_t address) const;

	[[nodiscard]] bool IsMemoryEmpty() const noexcept;

	[[nodiscard]] uint16_t GetStartAddress() const noexcept;

	[[nodiscard]] uint8_t Read(uint16_t address) const noexcept;
	void				  Write(uint16_t address, uint8_t value) noexcept;

	void Clear() noexcept;
	void RestoreSnapshot();

	void LoadValuesIntoMemory(const Program& program);
	void LoadProgramFromFIle(const std::filesystem::path& filename);

	uint8_t& operator[](uint16_t address);

private:
	MemoryUnit(const MemoryUnit&) = delete;
	MemoryUnit& operator=(const MemoryUnit&) = delete;

private:
	std::array<std::optional<uint8_t>, MEMORY_SIZE> m_memory{};

	bool m_hasData;
	std::vector<AddressValue> m_snapshotData;
	uint16_t m_startAddress{};
};

