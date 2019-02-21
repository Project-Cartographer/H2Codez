#include "SapienInterface.h"

bool SapienInterface::load_structure_bsp(int bsp_block_index, bool unload_old)
{
	static int load_structure_bsp_ptr = 0x4D6830;
	// char __usercall load_structure_bsp@<al>(int bsp_block_index@<ecx>, char unload_old)
	__asm {
		push unload_old
		mov ecx, bsp_block_index
		mov eax, load_structure_bsp_ptr
		call eax
		add esp, 4
	}
}
