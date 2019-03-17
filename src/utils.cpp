#include "utils.h"

bool InRange(int val, int lowerInclusive, int upperExclusive)
{
	return val >= lowerInclusive && val < upperExclusive;
}
