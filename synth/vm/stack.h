#pragma once

namespace invader {

class ZVMStack
{
public:

	ZVMStack(uintptr_t mem)
		: mem(mem)
		, offset(0)
	{
	}

	~ZVMStack(void)
	{
	}

	uintptr_t mem;
	uintptr_t offset;

	template <class T>
	void Push(const T& num)
	{
		*((T*)(mem+offset)) = num;
		offset += sizeof(T);
	}

	template <class T>
	T& Pop()
	{
		offset -= sizeof(T);
		return *((T*)(mem+offset));
	}
};

} // namespace invader
