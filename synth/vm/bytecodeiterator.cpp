#include "pch.h"

namespace invader {

ZVMBytecodeIterator::ZVMBytecodeIterator(const ZVMProgram& program)
	: program(program)
	, currentOpcode(0)
{

}

bool ZVMBytecodeIterator::Next(void)
{
	if (currentOpcode < program.bytecodeSize)
	{
		i = currentOpcode;
		opcode = program.bytecode[currentOpcode];
		argument = kInvalidArgument;

		if (opcode & kOpcodeMaskHasArgument)
		{
			argument = program.bytecode[currentOpcode+1];
			currentOpcode++;
		}

		currentOpcode++;

		return true;
	}
	else
	{
		opcode = kInvalidOpcode;
		argument = kInvalidArgument;
		i = kInvalidOpcodeIndex;
		return false;
	}
}

} // namespace invader