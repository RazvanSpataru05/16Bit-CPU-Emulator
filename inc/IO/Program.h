#pragma once

#include "Core/uint20_t.h"
#include "Core/MemoryCellType.h"

#include "IO/AddressValue.h"

#include <span>
#include <vector>

struct Program
{
	uint16_t startAddress{};
	std::vector<AddressValue> addrValues;
	std::unordered_map<uint20_t, MemoryCellType> memoryCellType;

	void AddMemoryCellTypes(uint20_t address, MemoryCellType cellType)
	{
		memoryCellType.insert({ address, cellType });
	}

	Program() {};
	Program(uint16_t startAddr, std::span<const AddressValue> addrVal,
		const std::unordered_map<uint20_t, MemoryCellType>& mem) :
		startAddress{ startAddr },
		memoryCellType{ mem }
	{
		addrValues.assign(addrVal.begin(), addrVal.end());
	}
};