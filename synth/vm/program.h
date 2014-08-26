#pragma once

namespace invader {

class ZVMProgram
{
public:
	ZVMProgram(void);

	static ZVMProgram* FromBlob(char* data) { return (ZVMProgram*)data; }
	ZVMProgram* Pack();
	ZVMProgram* Unpack();
	section_id_t GetConstGlobalSectionID()             { return section_id_t(0); }
	section_id_t GetGlobalSectionID()                  { return section_id_t(1); }
	section_id_t GetInstrumentSectionID(uint32_t i)    { return section_id_t(i+2); }
	section_id_t GetMasterSectionID()                  { return section_id_t(numSections-1); }

	size_t      programSize;
	size_t      bytecodeSize;
	size_t      globalStorageSize;
	size_t      numSections;
	opcode_t*   bytecode;
	label_t*    labels;
	label_t*    sections;
	num_t*      constants;
};

} // namespace invader