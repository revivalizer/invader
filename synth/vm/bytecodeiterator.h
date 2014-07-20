#pragma once

namespace invader {

#pragma warning(push)
#pragma warning(disable: 4512) // This is neccesary because we have a const object in class
class ZVMBytecodeIterator
{
public:
	ZVMBytecodeIterator(const ZVMProgram& program, opcode_index_t start = 0, opcode_index_t end = kInvalidOpcodeIndex);
	bool Next(void);

	opcode_t opcode; 

	opcode_index_t i;
	opcode_index_t start, end;
private:
	const ZVMProgram& program;
	opcode_index_t currentOpcode;
};
#pragma warning(pop)

} // namespace invader