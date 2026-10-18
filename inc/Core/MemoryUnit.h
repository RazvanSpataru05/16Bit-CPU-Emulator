#pragma once

#include "Core/uint20_t.h"
#include "Core/MemoryCellType.h"

#include "IO/DataLoader.h"
#include "IO/AddressValue.h"

#include "Utils/Constants.h"

#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <span>

class MemoryUnit
{
public:
	explicit MemoryUnit();

	MemoryUnit(MemoryUnit&&) = default;
	MemoryUnit& operator=(MemoryUnit&&) = default;

	const std::optional<uint8_t>& Peek(uint20_t address) const;

	[[nodiscard]] MemoryCellType GetMemoryCellType(uint20_t address) const noexcept;

	[[nodiscard]] bool IsMemoryEmpty() const noexcept;

	[[nodiscard]] uint16_t GetStartAddress() const noexcept;

	[[nodiscard]] uint8_t Read(uint20_t address) const noexcept;
	void Write(uint20_t address, uint8_t value);

	void Clear() noexcept;
	void RestoreSnapshot();

	void LoadValuesIntoMemory(const Program& program);
	void LoadProgramFromFIle(const std::filesystem::path& filename);

	const std::optional<uint8_t>& operator[](uint20_t address) const;
	std::optional<uint8_t>& operator[](uint20_t address);

private:
	MemoryUnit(const MemoryUnit&) = delete;
	MemoryUnit& operator=(const MemoryUnit&) = delete;

private:
	std::array<std::optional<uint8_t>, Utils::MEMORY_SIZE> m_memory{};
	std::unordered_map<uint20_t, MemoryCellType> m_memoryCellType{};

	bool m_hasData;
	std::vector<AddressValue> m_snapshotData;
	std::unordered_map<uint20_t, MemoryCellType> m_snapshotCellType;
	uint16_t m_startAddress{};
};

