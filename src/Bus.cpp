#include "bus.h"
#include "memory.h"

#include <fstream>
#include <assert.h>


u8 Bus::LoadU8(u16 address)
{
	// todo (chris) : target subsystem depending on address
	return Memory::LoadU8(address);
}

void Bus::StoreU8(u16 address, u8 val)
{
	// todo (chris) : target subsystem depending on address
	Memory::StoreU8(address, val);
}

