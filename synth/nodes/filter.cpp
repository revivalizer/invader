#include "pch.h"
#include "filter.h"

ZFilter::ZFilter(nodetype_t type) : ZSynthNode(type)
{
	filter.Reset();
}

void ZFilter::Process(ZVirtualMachine* vm, argument_t argument)
{
	vm; argument;
	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal& block = synthvm->blockStack->Top();
	
	uint32_t type    = static_cast<uint32_t>(vm->stack[-3]);
	double cutoff    = vm->stack[-2];
	double resonance = vm->stack[-1] / 100.0;
	vm->stack -= 3;

	if (argument == 1) // cutoff given in relative semitones to voice pitch
		cutoff = pitchToFrequency(synthvm->voice->pitch + cutoff);

	cutoff    = zclamp(cutoff, 5.0, kSampleRate*0.4);
	resonance = zclamp(resonance, 0.0, 1.0);

	filter.SetParameters(type, cutoff, resonance);
	filter.Process(block);
}

void ZFilter::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;
	filter.Reset();
}