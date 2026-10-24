#include "Core/MemoryUnit.h"

MemoryUnit::MemoryUnit() :
	m_hasData{ false },
	m_startAddress{ 0x0000 }
{
	std::fill(m_memory.begin(), m_memory.end(), std::nullopt);
}

const std::optional<uint8_t>& MemoryUnit::Peek(uint20_t address) const
{
	return m_memory[address.index()];
}

MemoryCellType MemoryUnit::GetMemoryCellType(uint20_t address) const noexcept
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

uint8_t MemoryUnit::Read(uint20_t address) const noexcept
{
	return m_memory[address.index()].value_or(0x00);
}

void MemoryUnit::Write(uint20_t address, uint8_t value)
{
	m_memory[address.index()].reset();
	m_memory[address.index()].emplace(value);
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
	std::copy(m_bytes.begin(), m_bytes.end(), m_memory.begin());

	//for (const auto& addrVal : m_snapshotData)
	//{
	//	m_memory[addrVal.address.index()] = addrVal.value;
	//	if (addrVal.value != 0x00)
	//	{
	//		m_hasData = true;
	//	}
	//}
	//m_memoryCellType = m_snapshotCellType;
}

void MemoryUnit::LoadProgramIntoMemory(const Program& program)
{
	if (program.startAddress + program.addrValues.size() > m_memory.size() || program.addrValues.empty()) return;

	m_snapshotData.assign(program.addrValues.begin(), program.addrValues.end());
	m_snapshotCellType = program.memoryCellType;

	for (const auto& addrVal : program.addrValues)
	{
		m_memory[addrVal.address.index()] = addrVal.value;
	}

	m_memoryCellType = program.memoryCellType;
	m_startAddress = program.startAddress;
	m_hasData = true;
}

void MemoryUnit::LoadBytesIntoMemory(const std::vector<uint8_t> bytes)
{
	std::copy(bytes.begin(), bytes.end(), m_memory.begin());
	m_bytes.insert(m_bytes.begin(), bytes.begin(), bytes.end());
	m_hasData = !bytes.empty();
}

void MemoryUnit::LoadProgramFromFIle(const std::filesystem::path& filename)
{
	const std::filesystem::path fullPath = std::filesystem::path("resources") / filename;
	//LoadValuesIntoMemory(DataLoader::ParseHexValues(fullPath, m_hasData), 0x0000);
}

const std::optional<uint8_t>& MemoryUnit::operator[](uint20_t address) const
{
	return m_memory[address.index()];
}

std::optional<uint8_t>& MemoryUnit::operator[](uint20_t address)
{
	return m_memory[address.index()];
}
