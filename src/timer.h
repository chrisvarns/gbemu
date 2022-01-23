#pragma once

#include "types.h"

namespace Timer
{
	void Init();
	void Step();


	u8 R_DIV();
	u8 R_TIMA();
	u8 R_TMA();
	u8 R_TAC();

	void W_DIV(u8 v);
	void W_TIMA(u8 v);
	void W_TMA(u8 v);
	void W_TAC(u8 v);
}