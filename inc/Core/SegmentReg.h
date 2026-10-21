#pragma once

#include <cstdint>

enum class SegmentReg : uint8_t
{
	ES = 0x00,			// Extra Segment
	CS,					// Code Segment
	DS,					// Data Segment
	SS					// Stack Segment
};