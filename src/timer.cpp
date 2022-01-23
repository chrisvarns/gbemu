#include <assert.h>
#include "timer.h"
#include "cpu.h"
#include "constants.h"


u16split divider;		// FF04 DIV
u8 timerCounter;		// FF05 TIMA
u8 timerModulo;			// FF06 TMA
u8 timerControl;		// FF07 TAC

int delayedInterupt = -1;


namespace Timer
{
	void IncrementTimer();
	bool TimerBit();
	void UpdateDivider(u16 value);


	void Init()
	{
		divider.Full = 0;
		timerCounter = 0;
		timerModulo = 0;
		timerControl = 0; // todo(luke) : what shoud the default value be?
	}

	void Step()
	{
		if (delayedInterupt > -1)
		{
			delayedInterupt--;
			if (delayedInterupt == -1)
			{
				timerCounter = timerModulo;
				CPU::RaiseInterrupt(INTERRUPT_FLAGS::TIMER);
			}
		}

		UpdateDivider(divider.Full + 1);
	}


	void IncrementTimer()
	{
		if (timerCounter == 0xFF)
		{
			timerCounter++;
			delayedInterupt = 4;
		}
		else
		{
			timerCounter++;
		}

	}

	bool TimerBit()
	{
		if (timerControl & 0x04)
		{
			int bit = 0;
			switch (timerControl & 0x03)
			{
			case 0: bit = 1 << 9; break;
			case 1: bit = 1 << 3; break;
			case 2: bit = 1 << 5; break;
			case 3: bit = 1 << 7; break;
			default:
				assert(false);
				break;
			}

			return divider.Full & bit;
		}
		else
		{
			return false;
		}
	}

	void UpdateDivider(u16 value)
	{
		int bit = 0;
		switch (timerControl & 0x03)
		{
		case 0: bit = 1 << 9; break;
		case 1: bit = 1 << 3; break;
		case 2: bit = 1 << 5; break;
		case 3: bit = 1 << 7; break;
		default:
			assert(false);
			break;
		}

		bool prev = TimerBit();
		divider.Full = value;
		bool post = TimerBit();

		if (prev == true && post == false)
		{
			IncrementTimer();
		}
	}


	u8 R_DIV() { return divider.H; }
	u8 R_TIMA() { return timerCounter; }
	u8 R_TMA() { return timerModulo; }
	u8 R_TAC() { return timerControl; }

	void W_DIV(u8 v) { UpdateDivider(0); }

	void W_TIMA(u8 v)
	{
		timerCounter = v;

		// writing to timer counter suppresses any pending overflow effects
		delayedInterupt = -1;
	}

	void W_TMA(u8 v)
	{
		timerModulo = v;

		// white the modulo is being loaded any writes changes the timer counter imidiately
		if (delayedInterupt > -1)
		{
			timerCounter = timerModulo;
		}
	}

	void W_TAC(u8 v)
	{
		bool prev = TimerBit();
		timerControl = 0;
		bool post = TimerBit();

		if (prev == false && post == true)
		{
			IncrementTimer();
		}
	}
}