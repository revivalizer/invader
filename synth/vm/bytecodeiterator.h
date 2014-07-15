#pragma once

namespace invader {

#pragma warning(push)
#pragma warning(disable: 4512) // This is neccesary because we have a const object in class
class ZVMBytecodeIterator
{
public:
	ZVMBytecodeIterator(const ZVMProgram& program);
	bool Next(void);

	opcode_t opcode; 
	argument_t argument;
	opcode_index_t i;
private:
	const ZVMProgram& program;
	opcode_index_t currentOpcode;
};
#pragma warning(pop)

} // namespace invader