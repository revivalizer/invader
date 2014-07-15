#pragma once

namespace invader {

class ZVMProgram
{
public:
	ZVMProgram(void);

	static ZVMProgram* FromBlob(char* data) { return (ZVMProgram*)data; }
	ZVMProgram* Pack();
	ZVMProgram* Unpack();

	size_t      programSize;
	size_t      bytecodeSize;
	size_t      globalStorageSize;
	size_t		numInstruments;
	opcode_t*   bytecode;
	label_t*    labels;
	num_t*      constants;
};

} // namespace invader