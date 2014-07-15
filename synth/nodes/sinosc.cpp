#include "pch.h"
#include "sinosc.h"

ZSinOsc::ZSinOsc(nodetype_t type) : ZSynthNode(type)
	, phase(0)
{

}

void ZSinOsc::Process(ZVirtualMachine* vm, argument_t argument)
{
	argument;

	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal block;

	double detune = vm->stack[-1];
	vm->stack--;

	double deltaPhase = kM_PI2*pitchToFrequency(synthvm->voice->pitch + detune)/kSampleRate;

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i].v = _mm_set1_pd(zsind(phase));

		phase += deltaPhase;
	}

	synthvm->blockStack->Push(block);
}