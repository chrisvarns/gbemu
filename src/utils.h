#pragma once
#include "constants.h"

inline bool InRange(int val, int lowerInclusive, int upperExclusive)
{
	return val >= lowerInclusive && val < upperExclusive;
}

inline bool InRange(int val, AddressRegion lowerInclusive, AddressRegion upperExclusive)
{
	return InRange(val, (int)lowerInclusive, (int)upperExclusive);
}