#include "pch.h"
#include "mixer.h"

ZMixer::ZMixer(nodetype_t type) : ZSynthNode(type)
{
}

void ZMixer::Process(ZVirtualMachine* vm, argument_t numInputs)
{
	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal block;
	block.Reset();

	for (uint32_t input=0; input<numInputs; input++)
	{
		double g = vm->stack[-1];
		vm->stack--;

		sample_t gain = sample_t(dbToGain(g));
		ZBlockBufferInternal& in = synthvm->blockStack->Pop();

		for (uint32_t i=0; i<block.numSamples; i++)
		{
			block.samples[i] += in.samples[i]*gain;
		}
	}

	synthvm->blockStack->Push(block);
}
