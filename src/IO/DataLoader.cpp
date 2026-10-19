#include "IO/DataLoader.h"
#include "Assembler/Assembler.h"

std::vector<uint8_t> DataLoader::ParseHexValues(const std::filesystem::path& path, bool& hasData)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Error: could not open file " + path.string());
	}

	std::vector<uint8_t> values;
	unsigned hexValue{};

	while (file >> std::hex >> hexValue)
	{
		values.emplace_back(static_cast<uint8_t>(hexValue));
		if (hexValue != 0x00)
		{
			hasData = true;
		}
	}

	file.close();
	return values;
}

Program DataLoader::ParseStatements(std::span<const Statement> statements, uint16_t startAddress)
{
	std::vector<AddressValue> values;
	Program program;
	program.startAddress = startAddress;

	for (const auto& statement : statements)
	{
		uint20_t address = statement.address;

		// this is an instruction
		for (uint8_t byte : Assembler::Encode(statement))
		{
			++address;
			program.addrValues.push_back({ address, byte });
			program.memoryCellType.insert({ address, MemoryCellType::DATA });
		}
	}
	if (values.size() < Utils::Constants::MEMORY_SIZE)
	{
		return program;
	}
	return Program();
}
