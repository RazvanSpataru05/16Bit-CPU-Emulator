#pragma once

#include "Assembler/Assembler.h"

#include "Debugger/Dissasembler.h"

class Toolchain
{
public:
	Toolchain();

	Toolchain(Toolchain&&) = default;
	Toolchain& operator=(Toolchain&&) = default;

	[[nodiscard]] const Assembler&	GetAssembler()				const noexcept;
	[[nodiscard]] Assembler&		GetAssembler()				noexcept;

	[[nodiscard]] const Dissasembler&	GetDissasembler() const noexcept;
	[[nodiscard]] Dissasembler&			GetDissasembler() noexcept;

private:
	Toolchain(const Toolchain&) = delete;
	Toolchain& operator=(const Toolchain&) = delete;

private:
	Assembler m_assembler;
	Dissasembler m_dissasembler;
};