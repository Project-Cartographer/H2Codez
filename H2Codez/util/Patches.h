#pragma once

/*
	Writes `numBytes` bytes from `patch` to `destAddress`
*/
inline void WriteBytes(void* destAddress, void *patch, DWORD numBytes)
{
	DWORD OldProtection;

	VirtualProtect(destAddress, numBytes, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy(destAddress, patch, numBytes);
	VirtualProtect(destAddress, numBytes, OldProtection, NULL);

	FlushInstructionCache(GetCurrentProcess(), destAddress, numBytes);
}

/*
	Writes `numBytes` bytes from `patch` to `destAddress`
*/
inline void WriteBytes(DWORD destAddress, void *patch, DWORD numBytes)
{
	WriteBytes(reinterpret_cast<void*>(destAddress), patch, numBytes);
}

/*
	Writes data to memory at address
*/
template <typename value_type>
inline void WriteValue(DWORD address, value_type data)
{
	WriteBytes(address, &data, sizeof(data));
}




/*
	Writes pointer to memory address
*/
inline void WritePointer(DWORD offset, const void *ptr) {
	WriteValue(offset, ptr);
}

/*
	Writes pointer to memory address
*/
inline void WritePointer(void *offset, const void *ptr) {
	WriteValue(reinterpret_cast<DWORD>(offset), ptr);
}




/*
	Write a block of `len` of nops at `address`
*/
inline void NopFill(const DWORD address, int len)
{
	BYTE *nop_fill = new BYTE[len];
	memset(nop_fill, 0x90, len);
	WriteBytes(address, nop_fill, len);

	delete[] nop_fill;
}

/*
	Write a block of `len` of nops at `address`
*/
inline void NopFill(const void *address, int len)
{
	NopFill(reinterpret_cast<DWORD>(address), len);
}

inline void NopFillRange(const DWORD address_start, const DWORD address_end)
{
	NopFill(address_start, address_end - address_start);
}

inline void NopFillRange(const void *address_start, const void *address_end)
{
	NopFillRange(reinterpret_cast<DWORD>(address_start), reinterpret_cast<DWORD>(address_end));
}




/*
	Patches an existing function call
*/
inline void PatchCall(DWORD call_addr, DWORD new_function_ptr) {
	DWORD callRelative = new_function_ptr - (call_addr + 5);
	WriteValue(call_addr + 1, reinterpret_cast<void*>(callRelative));
}

/*
	Patches an existing function call
*/
inline void PatchCall(DWORD call_addr, void *new_function_ptr)
{
	PatchCall(call_addr, reinterpret_cast<DWORD>(new_function_ptr));
}



/*
	Write relative jump at `address` to `target_addr`
*/
inline void WriteJmp(DWORD call_addr, DWORD target_addr)
{
	BYTE call_patch[1] = { 0xE9 };
	WriteBytes(call_addr, call_patch, 1);
	PatchCall(call_addr, target_addr);
}

/*
	Write relative jump at `address` to `target_addr`
*/
inline void WriteJmp(DWORD address, void *target_addr)
{
	WriteJmp(address, reinterpret_cast<DWORD>(target_addr));
}

/*
	Write relative jump at `address` to `target_addr`
*/
inline void WriteJmp(void *address, void *target_addr)
{
	WriteJmp(reinterpret_cast<DWORD>(address), reinterpret_cast<DWORD>(target_addr));
}



/*
	Write call to `function_ptr` at `address`
*/
inline void WriteCall(DWORD address, DWORD function_ptr)
{
	BYTE call_patch[1] = { 0xE8 };
	WriteBytes(address, call_patch, 1);
	PatchCall(address, function_ptr);
}

/*
	Write call to `function_ptr` at `address`
*/
inline void WriteCall(DWORD address, void *function_ptr)
{
	WriteCall(address, reinterpret_cast<DWORD>(function_ptr));
}

/*
	Write call to `function_ptr` at `address`
*/
inline void WriteCall(void *address, void *function_ptr)
{
	WriteCall(reinterpret_cast<DWORD>(address), reinterpret_cast<DWORD>(function_ptr));
}



/*
	Patches an absolute call
*/
inline void PatchAbsCall(DWORD call_addr, DWORD new_function_ptr)
{
	WriteCall(call_addr, new_function_ptr);
	NopFill(call_addr + 5, 1);
}

/*
	Patches an absolute call
*/
inline void PatchAbsCall(void *call_addr, void *new_function_ptr)
{
	PatchAbsCall(reinterpret_cast<DWORD>(call_addr), reinterpret_cast<DWORD>(new_function_ptr));
}

/*
	Patches an absolute call
*/
inline void PatchAbsCall(DWORD call_addr, void *new_function_ptr)
{
	PatchAbsCall(call_addr, reinterpret_cast<DWORD>(new_function_ptr));
}

/*
	Patches an absolute call
*/
inline void PatchAbsCall(void *call_addr, DWORD new_function_ptr)
{
	PatchAbsCall(reinterpret_cast<DWORD>(call_addr), new_function_ptr);
}
