#include "pch.h"
#include "adsrgain.h"

namespace invader { 

ZADSRGain::ZADSRGain(nodetype_t type) : ZNode(type)
	, didNoteOn(false)
	, didNoteOff(false)
{
	env.attackShape   = 0.8;
	env.decayShape    = 0.2;
	env.releaseShape  = 0.8;

	env.fade = 0.0;
	env.hold = 0.0;

	env.type = kEnvelopeTypeDigital;

	env.releaseMode  = kEnvelopeReleaseModeHard;
	env.fadeHoldMode = kEnvelopeFadeHoldModeOff;
	env.delayMode    = kEnvelopeDelayModeOff;

	env.noteOnDelay  = 0.0;
	env.noteOffDelay = 0.0;

	filter.SetCutoff(375.0/4.0);
}

void ZADSRGain::Process(ZVirtualMachine* vm)
{
	release = vm->stack->Pop<num_t>();
	sustain = vm->stack->Pop<num_t>();
	decay   = vm->stack->Pop<num_t>();
	attack  = vm->stack->Pop<num_t>();

	if (didNoteOn)
	{
		env.attack = attack;
		env.decay = decay;
		env.sustain = dbToGain(sustain);

		env.NoteOn();
		didNoteOn = false;
	}
	if (didNoteOff)
	{
		env.release = release;

		env.NoteOff();
		didNoteOff = false;
	}

	sample_t gain = sample_t(env.Advance(kBlockSize*kDefaultOversampling));
	ZBlockBufferInternal& block = vm->stack->Pop<ZBlockBufferInternal>();

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i] *= filter.Lowpass(gain);
	}

	vm->stack->Push(block);
}

void ZADSRGain::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;

	didNoteOn = true;

	filter.Reset();
}

void ZADSRGain::NoteOff(uint32_t deltaSamples)
{
	deltaSamples;

	didNoteOff = true;
}

} // namespace invader