#pragma once

class c_allocation_base
{
public:
	virtual void *allocate(size_t size, char* file, size_t line, const char *type, const char *pool_name, const char *name) = 0;
	virtual void free(void *pointer, char* file, size_t line) = 0;
};
static_assert(sizeof(c_allocation_base) == 4, "bad c_allocation_base size (vtable missing?)");

class c_no_allocation : c_allocation_base
{
public:
	virtual void *allocate(size_t size, char* file, size_t line, const char *type, const char *pool_name, const char *name) { return nullptr; }
	virtual void free(void *pointer, char* file, size_t line) {};
};
static_assert(sizeof(c_no_allocation) == 4, "bad c_no_allocation size (vtable missing?)");

/* 
	Interface for the h2EK debug allocator
	You need to use this to manage memory if you want to compatibility with some EK functions.
*/
namespace debug_memory
{
	inline void *allocate(
		const char* file,
		int line,
		size_t size,
		char alignment = 0,
		const char *type = nullptr,
		const char *subtype = nullptr,
		const char *name = nullptr
		)
	{
		typedef wchar_t* __cdecl debug_malloc(size_t size, char alignment, const char *file, int line, const void *type, const void *subtype, const void *name);
		auto debug_malloc_impl = reinterpret_cast<debug_malloc*>(SwitchByMode(0x52B540u, 0u, 0u));
		CHECK_FUNCTION_SUPPORT(debug_malloc_impl);
		return debug_malloc_impl(size, alignment, file, line, type, subtype, name);
	}

	inline void *allocate(
		size_t size,
		char alignment = 0,
		const char *type = nullptr,
		const char *subtype = nullptr,
		const char *name = nullptr
	)
	{
		return allocate(__FILE__, __LINE__, size, alignment, type, subtype, name);
	}

	inline void *reallocate(
		const char* file,
		int line,
		void* old_pointer,
		size_t size,
		char alignment = 0,
		const char *type = nullptr,
		const char *subtype = nullptr,
		const char *name = nullptr
		)
	{
		typedef wchar_t* __cdecl debug_realloc(void *old_pointer, size_t size, char alignment, const char *file, int line, const void *type, const void *subtype, const void *name);
		auto debug_realloc_impl = reinterpret_cast<debug_realloc*>(SwitchByMode(0x52B7A0u, 0u, 0u));
		CHECK_FUNCTION_SUPPORT(debug_realloc_impl);
		return debug_realloc_impl(old_pointer, size, alignment, file, line, type, subtype, name);
	}

	inline void *reallocate(
		void* old_pointer,
		size_t size,
		char alignment = 0,
		const char *type = nullptr,
		const char *subtype = nullptr,
		const char *name = nullptr
	)
	{
		return reallocate(__FILE__, __LINE__, old_pointer, size, alignment, type, subtype, name);
	}

	inline void free(void *pointer, const char *file)
	{
		typedef void __cdecl debug_free(void* pointer, const char* file);
		auto debug_free_impl = reinterpret_cast<debug_free*>(SwitchByMode(0x52B040u, 0u, 0u));
		CHECK_FUNCTION_SUPPORT(debug_free_impl);
		debug_free_impl(pointer, file);
	}

	inline void free(void* pointer)
	{
		free(pointer, __FILE__);
	}
}

/* 
	Allocates memory using the H2EK debug allocator
	See debug_memory::allocate for full list of args
 */
#define HEK_DEBUG_MALLOC(size, ...) \
	debug_memory::allocate( __FILE__, __LINE__, size, __VA_ARGS__)

 /*
	 Allocates memory array using the H2EK debug allocator
	 See debug_memory::allocate for full list of args
  */
#define HEK_DEBUG_NEW(type, length, ...) \
	reinterpret_cast<type*>(debug_memory::allocate( __FILE__, __LINE__, length * sizeof(type), __VA_ARGS__))

// Re-allocates memory allocated with debug_memory::allocate
#define HEK_DEBUG_REALLOC(pointer, size, ...) \
	debug_memory::reallocate( __FILE__, __LINE__, size, __VA_ARGS__)

// Frees memory allocated with debug_memory::allocate
#define HEK_DEBUG_FREE(pointer)\
	debug_memory::free(pointer, __FILE__);