#include "Core/Toolchain.h"

Toolchain::Toolchain() :
	m_assembler{ Assembler() },
	m_dissasembler{ Dissasembler() }
{
}

const Assembler& Toolchain::GetAssembler() const noexcept
{
	return m_assembler;
}

Assembler& Toolchain::GetAssembler() noexcept
{
	return m_assembler;
}

const Dissasembler& Toolchain::GetDissasembler() const noexcept
{
	return m_dissasembler;
}

Dissasembler& Toolchain::GetDissasembler() noexcept
{
	return m_dissasembler;
}
