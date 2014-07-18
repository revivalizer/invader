#pragma once

namespace invader {

class ZSynth;
class ZInstrument;
class ZVoice;

class ZVirtualMachine
{
public:
	ZVirtualMachine(ZVMProgram* program, ZVMStack* stack);
	~ZVirtualMachine(void);

	void Run(opcode_t ip, ZVMProgram* program);

	ZNode* CreateNodeFromOpcode(nodetype_t type);
	
	void CreateNodeInstances(ZVMProgram* program, opcode_index_t start, opcode_index_t end);

	ZNode**               nodeInstances;
	ZVMStack*             stack;
	ZVMStorage            globalStorage;

	const ZVMProgram* program;

	// Context
	ZSynth*      synth;
	ZInstrument* instrument;
	ZVoice*      voice;
};

} // namespace invader