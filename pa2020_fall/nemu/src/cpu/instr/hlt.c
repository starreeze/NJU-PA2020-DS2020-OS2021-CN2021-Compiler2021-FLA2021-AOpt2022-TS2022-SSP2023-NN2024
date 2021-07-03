#include "cpu/instr.h"
#include <unistd.h>

make_instr_func(hlt)
{
#ifdef MYDEBUG
    printf("\033[43m\033[30mhit \'hlt\' at eip = 0x%x\033[0m\n", cpu.eip);
#endif
	print_asm_0("hlt", "", 1);
#ifdef HAS_DEVICE_TIMER
	is_nemu_hlt = true;
#endif
	return 1;
}
