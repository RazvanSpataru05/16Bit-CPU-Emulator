#pragma once

#include <cstdint>

enum class SegmentOverride : uint8_t
{
	NONE = 0u,
	ES,					// Extra Segment
	CS,					// Code Segment
	SS,					// Stack Segment
	DS					// Data Segment
};