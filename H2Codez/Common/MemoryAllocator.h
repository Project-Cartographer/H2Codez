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
