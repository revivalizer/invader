#include "pch.h"

namespace invader {

ZVMProgram::ZVMProgram(void)
	: bytecode(NULL)
{
}

ZVMProgram* ZVMProgram::Unpack()
{
	bytecode   = (opcode_t*)  ((uintptr_t)bytecode   + (uintptr_t)this);
	labels     = (opcode_t*)  ((uintptr_t)labels     + (uintptr_t)this);
	//numconstants  = (constant_t*)((uintptr_t)numconstants  + (uintptr_t)this);

	return this;
}

ZVMProgram* ZVMProgram::Pack()
{
	bytecode   = (opcode_t*)  ((uintptr_t)bytecode   - (uintptr_t)this);
	labels     = (opcode_t*)  ((uintptr_t)labels     - (uintptr_t)this);
	//numconstants  = (constant_t*)((uintptr_t)numconstants  - (uintptr_t)this);

	return this;
}

} // namespace invader

