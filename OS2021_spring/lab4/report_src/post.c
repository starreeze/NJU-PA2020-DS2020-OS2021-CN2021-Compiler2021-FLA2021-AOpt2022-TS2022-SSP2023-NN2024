void syscallSemPost(struct StackFrame *sf)
{
    int i = (int)sf->edx;
    if (i < 0 || i >= MAX_SEM_NUM)
    {
        pcb[current].regs.eax = -1;
        return;
    }
    if (++sem[i].value <= 0)
    {
        ProcessTable *pt = pop_sem(i);
        // if (!--pt->blocked_sems)
        pt->state = STATE_RUNNABLE;
    }
    sf->eax = 0;
}