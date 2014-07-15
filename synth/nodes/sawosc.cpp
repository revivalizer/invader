#include "pch.h"
#include "sawosc.h"

ZSawOsc::ZSawOsc(nodetype_t type) : ZSynthNode(type)
	, phase(0)
{

}

void ZSawOsc::Process(ZVirtualMachine* vm, argument_t argument)
{
	argument;

	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal block;

	double detune = vm->stack[-1];
	vm->stack--;

	double deltaPhase = 2.0*pitchToFrequency(synthvm->voice->pitch + detune)/kSampleRate;

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i].v = _mm_set1_pd(phase);

		phase += deltaPhase;
		if (phase > 1.0)
			phase -= 2.0;
	}

	synthvm->blockStack->Push(block);
}