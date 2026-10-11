#include "IO/DataLoader.h"

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
		uint16_t address = statement.address;

		// this is an instruction
		if (statement.opcode.has_value()) 
		{
			program.addrValues.push_back({ address, statement.opcode.value() });
			program.AddMemoryCellTypes(address, MemoryCellType::OPCODE);
			++address;

			for (size_t index = 0; index < statement.operatorCount; ++index)
			{
				program.addrValues.push_back({ address, statement.operands[index] });
				program.AddMemoryCellTypes(address, MemoryCellType::OPERAND);
				++address;
			}
		}
		else
		{
			for (size_t index = 0; index < statement.operatorCount; ++index)
			{
				program.addrValues.push_back({ address, statement.operands[index] });
				program.AddMemoryCellTypes(address, MemoryCellType::DATA);
				++address;
			}
		}


	}
	if (values.size() < std::numeric_limits<uint16_t>::max())
	{
		return program;
	}
	return Program();
}
