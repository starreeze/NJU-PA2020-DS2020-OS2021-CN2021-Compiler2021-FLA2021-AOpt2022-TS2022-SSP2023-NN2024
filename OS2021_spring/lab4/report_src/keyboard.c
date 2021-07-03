void keyboardHandle(struct StackFrame *sf)
{
	uint32_t keyCode = getKeyCode();
	if (keyCode == 0) // illegal keyCode
		return;

	keyBuffer[bufferTail] = keyCode;
	bufferTail = (bufferTail + 1) % MAX_KEYBUFFER_SIZE;

	assert(dev[STD_IN].value == -1 || dev[STD_IN].value == 0);
	if (dev[STD_IN].value < 0)
	{ // with process blocked
		// TODO: deal with blocked situation
		ProcessTable *pt = pop_dev(STD_IN);
		++dev[STD_IN].value;
		pt->state = STATE_RUNNABLE;
	}

	return;
}