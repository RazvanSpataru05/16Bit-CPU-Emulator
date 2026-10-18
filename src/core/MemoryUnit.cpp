#include "Core/MemoryUnit.h"

MemoryUnit::MemoryUnit() :
	m_hasData{ false },
	m_startAddress{ 0x0000 }
{
	std::fill(m_memory.begin(), m_memory.end(), std::nullopt);
}

const std::optional<uint16_t>& MemoryUnit::Peek(uint16_t address) const
{
	return m_memory[address];
}

MemoryCellType MemoryUnit::GetMemoryCellType(uint16_t address) const noexcept
{
	const auto it = m_memoryCellType.find(address);

	if (it == m_memoryCellType.end())
	{
		return MemoryCellType::UNKNOWN;
	}
	return it->second;
}

bool MemoryUnit::IsMemoryEmpty() const noexcept
{
	return !m_hasData;
}

uint16_t MemoryUnit::GetStartAddress() const noexcept
{
	return m_startAddress;
}

uint16_t MemoryUnit::Read(uint16_t address) const noexcept
{
	return m_memory[address].value_or(0x00);
}

void MemoryUnit::Write(uint16_t address, uint16_t value)
{
	std::cout << std::hex
		<< "address = " << address
		<< ", size = " << m_memory.size()
		<< ", value = " << static_cast<int>(value)
		<< '\n';

	m_memory[address].reset();
	m_memory[address].emplace(value);
	m_hasData = true;
}

void MemoryUnit::Clear() noexcept
{
	std::fill(m_memory.begin(), m_memory.end(), std::nullopt);
	m_memoryCellType.clear();
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
	m_memoryCellType = m_snapshotCellType;
}

void MemoryUnit::LoadValuesIntoMemory(const Program& program)
{
	if (program.startAddress + program.addrValues.size() > m_memory.size() || program.addrValues.empty()) return;

	m_snapshotData.assign(program.addrValues.begin(), program.addrValues.end());
	m_snapshotCellType = program.memoryCellType;

	for (const auto& addrVal : program.addrValues)
	{
		m_memory[addrVal.address] = addrVal.value;
	}

	m_memoryCellType = program.memoryCellType;
	m_startAddress = program.startAddress;
	m_hasData = true;
}

void MemoryUnit::LoadProgramFromFIle(const std::filesystem::path& filename)
{
	const std::filesystem::path fullPath = std::filesystem::path("resources") / filename;
	//LoadValuesIntoMemory(DataLoader::ParseHexValues(fullPath, m_hasData), 0x0000);
}

const std::optional<uint8_t>& MemoryUnit::operator[](uint16_t address) const
{
	return m_memory[address];
}

std::optional<uint8_t>& MemoryUnit::operator[](uint16_t address)
{
	return m_memory[address];
}
