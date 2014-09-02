#include "pch.h"
#include "wavetableosc.h"

namespace invader { 

ZWavetableOsc::ZWavetableOsc(nodetype_t type) : ZNode(type)
{
}

void ZWavetableOsc::Process(ZVirtualMachine* vm)
{
//	auto frequency = vm->stack->Pop<num_t>();

	ZBlockBufferInternal block;

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		double time = vm->voice->timeSinceNoteOn + i/kSampleRate;
		block.samples[zifloord(i)] = sample_t(zsind(time*pitchToFrequency(vm->voice->pitch)*kM_PI2));
	}

	vm->stack->Push(block);
}

void ZWavetableOsc::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;
}

void ZWavetableOsc::NoteOff(uint32_t deltaSamples)
{
	deltaSamples;
}

} // namespace invader