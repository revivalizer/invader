#include "pch.h"

namespace invader {

template <typename T, typename U>
T* ZPack(T* a, U* b)
{
	return (T*)((uintptr_t)a-(uintptr_t)b);
}

template <typename T, typename U>
T* ZUnpack(T* a, U* b)
{
	return (T*)((uintptr_t)a+(uintptr_t)b);
}

ZVMProgram::ZVMProgram(void)
	: bytecode(NULL)
{
}

ZVMProgram* ZVMProgram::Unpack()
{
	bytecode  = ZUnpack(bytecode, this);
	labels    = ZUnpack(labels, this);
	sections  = ZUnpack(sections, this);
	constants = ZUnpack(constants, this);

	return this;
}

ZVMProgram* ZVMProgram::Pack()
{
	bytecode  = ZPack(bytecode, this);
	labels    = ZPack(labels, this);
	sections  = ZPack(sections, this);
	constants = ZPack(constants, this);

	return this;
}

} // namespace invader

