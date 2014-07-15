#include "pch.h"
#include "adsrgain.h"

ZADSRGain::ZADSRGain(nodetype_t type) : ZSynthNode(type)
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

void ZADSRGain::Process(ZVirtualMachine* vm, argument_t argument)
{
	argument;
	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	attack  = vm->stack[-4];
	decay   = vm->stack[-3];
	sustain = vm->stack[-2];
	release = vm->stack[-1];

	vm->stack -= 4;

	if (didNoteOn)
	{
		env.attack = attack/1000.0;
		env.decay = decay/1000.0;
		env.sustain = dbToGain(sustain);

		env.NoteOn();
		didNoteOn = false;
	}
	if (didNoteOff)
	{
		env.release = release/1000.0;

		env.NoteOff();
		didNoteOff = false;
	}

	sample_t gain = sample_t(env.Advance(kBlockSize*kDefaultOversampling));
	ZBlockBufferInternal& block = synthvm->blockStack->Top();

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i] *= filter.Lowpass(gain);
		//block.samples[i] *= gain;
	}
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