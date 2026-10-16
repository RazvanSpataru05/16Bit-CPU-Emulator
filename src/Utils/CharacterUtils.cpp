#include "Utils/CharacterUtils.h"

namespace Utils
{
    namespace Character
    {
        bool IsValidDigit(unsigned char c, uint16_t base)
        {
            switch (base)
            {
            case 2:
                return c == '0' || c == '1';

            case 10:
                return std::isdigit(c);

            case 16:
                return std::isxdigit(c);

            default:
                return false;
            }
        }
    }
}


