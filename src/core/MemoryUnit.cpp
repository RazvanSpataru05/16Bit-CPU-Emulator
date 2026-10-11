#include "Core/MemoryUnit.h"

MemoryUnit::MemoryUnit() :
	m_hasData{ false },
	m_startAddress{ 0x0000 }
{
	std::fill(m_memory.begin(), m_memory.end(), std::nullopt);
}

const std::optional<uint8_t>& MemoryUnit::Peek(uint16_t address) const
{
	return m_memory[address];
}

bool MemoryUnit::IsMemoryEmpty() const noexcept
{
	return !m_hasData;
}
uint16_t MemoryUnit::GetStartAddress() const noexcept
{
	return m_startAddress;
}

uint8_t MemoryUnit::Read(uint16_t address) const noexcept
{
	return m_memory[address].value_or(0x00);
}

void MemoryUnit::Write(uint16_t address, uint8_t value) noexcept
{
	m_memory[address] = value;
	if (value != 0x00)
	{
		m_hasData = true;
	}
}

void MemoryUnit::Clear() noexcept
{
	std::fill(m_memory.begin(), m_memory.end(), std::nullopt);
	m_hasData = false;
}

void MemoryUnit::RestoreSnapshot()
{
	Clear();
	for (const auto& addrVal : m_snapshotData)
	{
		m_memory[addrVal.address] = addrVal.value;
		if (addrVal.value != 0x00)
		{
			m_hasData = true;
		}
	}
}

void MemoryUnit::LoadValuesIntoMemory(const Program& program)
{
	if (program.startAddress + program.addrValues.size() > m_memory.size() || program.addrValues.empty()) return;

	m_snapshotData.assign(program.addrValues.begin(), program.addrValues.end());

	for (const auto& addrVal : program.addrValues)
	{
		m_memory[addrVal.address] = addrVal.value;
		if (addrVal.value != 0x00)
		{
			m_hasData = true;
		}
	}

	m_startAddress = program.startAddress;

	m_hasData = true;
}

void MemoryUnit::LoadProgramFromFIle(const std::filesystem::path& filename)
{
	const std::filesystem::path fullPath = std::filesystem::path("resources") / filename;
	//LoadValuesIntoMemory(DataLoader::ParseHexValues(fullPath, m_hasData), 0x0000);
}

uint8_t& MemoryUnit::operator[](uint16_t address) 
{
	if (!m_memory[address].has_value())
	{
		m_memory[address] = 0x00;
	}

	return m_memory[address].value();
}
