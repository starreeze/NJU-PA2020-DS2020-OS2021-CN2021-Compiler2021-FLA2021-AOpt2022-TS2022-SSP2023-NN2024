#include "hal.h"

static volatile uint32_t jiffy = 0;
static int fps = 0;
static int nr_draw = 0;

void incr_nr_draw(void)
{
	nr_draw++;
}

int get_fps()
{
	return fps;
}

void timer_event(void)
{
	jiffy++;
	if (jiffy % (HZ / 2) == 0)
	{
		fps = nr_draw * 2 + 1;
		nr_draw = 0;
	}
}

uint32_t SDL_GetTicks()
{
	/* TODO: Return the time in millisecond. */
	return jiffy * 10;
}

void SDL_Delay(uint32_t ms)
{
	for(int i = 0; i < ms/10; ++i)
		timer_event();
}
