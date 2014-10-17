#pragma once

namespace invader {

class ZSynth;
class ZInstrument;
class ZVoice;

class ZVirtualMachine
{
public:
	ZVirtualMachine(ZVMProgram* program, ZVMStack* stack, ZVMStorage* storage);
	~ZVirtualMachine(void);

	void Run(opcode_t ip, ZVMProgram* program);

	ZNode* CreateNodeFromOpcode(nodetype_t type);
	
	void CreateNodeInstances(ZVMProgram* program, uint32_t section);

	ZNode**               nodeInstances;
	ZVMStack*             stack;
	ZVMStorage*           globalStorage;

	const ZVMProgram* program;

	// Context
	ZSynth*      synth;
	ZInstrument* instrument;
	ZVoice*      voice;

	// Temp vars (so we don't allocate them on stack frame)
	ZRealSpectrum tempSpec;
	ZBlockBufferInternal tempBlock;
};

} // namespace invader