#include "pch.h"

namespace invader {

	ZVMBytecodeIterator::ZVMBytecodeIterator( const ZVMProgram& program, opcode_index_t start, opcode_index_t end ) : program(program)
		, currentOpcode(start)
		, start(start)
		, end(end)
	{

	if (end==kInvalidOpcodeIndex)
		end = (opcode_index_t)program.bytecodeSize;
}

bool ZVMBytecodeIterator::Next(void)
{
	if (currentOpcode < end)
	{
		i = currentOpcode;
		opcode = program.bytecode[currentOpcode];

		switch (opcode & (!0xF))
		{
			case kOpPush:
			case kOpPushGlobal:
			case kOpPopGlobal:
			case kOpCallFunc:
			case kOpJump:
			case kOpJumpEqual:
			case kOpJumpNotEqual:
				currentOpcode+=2;
				break;

			case kOpReturn:
			case kOpPop:
			case kOpDec:
			case kOpClone:
			case kOpAdd:
			case kOpSubtract:
			case kOpMultiply:
			case kOpDivide:
			case kOpModulo:
			case kOpEqual:
			case kOpNotEqual:
			case kOpLessThan:
			case kOpLessThanOrEqual:
			case kOpGreaterThan:
			case kOpGreaterThanOrEqual:
			case kOpLogicalAnd:
			case kOpLogicalOr:
			case kOpNot:
			case kOpPlus:
			case kOpMinus:
				currentOpcode+=1;
				break;

			default:
				MessageBoxA(nullptr, "Unhandled Case in ZVMBytecodeIterator", "Fatal Error", 0);
				ExitProcess(9);
				break;
		}

		return true;
	}
	else
	{
		opcode = kInvalidOpcode;
		i = kInvalidOpcodeIndex;
		return false;
	}
}

} // namespace invader