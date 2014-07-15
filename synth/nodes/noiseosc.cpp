#include "pch.h"
#include "noiseosc.h"

ZNoiseOsc::ZNoiseOsc(nodetype_t type) : ZSynthNode(type)
	, rand(0x8f3a772b)
{

}

void ZNoiseOsc::Process(ZVirtualMachine* vm, argument_t argument)
{
	argument;

	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal block;

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i].v = _mm_set1_pd(rand.NextUniformFloat()*2.0-1.0);
	}

	synthvm->blockStack->Push(block);
}