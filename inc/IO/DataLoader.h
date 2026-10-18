#pragma once

#include "Assembler/Statement.h"

#include "Utils/Logger.h"
#include "Utils/Constants.h"

#include "IO/Program.h"

#include <filesystem>
#include <vector>
#include <fstream>
#include <span>

class Assembler;

namespace DataLoader
{
	std::vector<uint8_t> ParseHexValues(const std::filesystem::path& path, bool& hasData);
	Program ParseStatements(std::span<const Statement> statements, uint16_t startAddress);
};