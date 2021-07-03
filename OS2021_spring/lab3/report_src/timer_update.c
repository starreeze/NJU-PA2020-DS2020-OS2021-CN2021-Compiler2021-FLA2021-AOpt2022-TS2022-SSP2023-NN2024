for (int i = 0; i < MAX_PCB_NUM; ++i)
{
	if (pcb[i].state == STATE_BLOCKED && !--pcb[i].sleepTime)
	{
		pcb[i].state = STATE_RUNNABLE;
	}
}
if (pcb[current].state != STATE_RUNNING || ++pcb[current].timeCount >= MAX_TIME_COUNT)
{
	int next = 0;
	for (int i = 1; i < MAX_PCB_NUM; ++i)
		if (pcb[i].state == STATE_RUNNABLE)
		{
			next = i;
			break;
		}
}