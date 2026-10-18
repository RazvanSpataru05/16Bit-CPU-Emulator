#pragma once

#include <cstdint>

namespace ISA
{
    enum class OperandKind : uint8_t
    {
        NONE,
        REG_8, // lo or hi of register
        REG_16, //  16-bit GP register

        SEG, // segment register

        IMM_8, 
        IMM_16,

        MEM, // value from memory unit

        REL_8,
        REL_16,

        REG_REG
    };
}
