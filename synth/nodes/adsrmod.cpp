#include "pch.h"
#include "adsrmod.h"

namespace invader { 

ZADSRMod::ZADSRMod(nodetype_t type) : ZNode(type)
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

void ZADSRMod::Process(ZVirtualMachine* vm)
{
	release = vm->stack->Pop<num_t>();
	sustain = vm->stack->Pop<num_t>();
	decay   = vm->stack->Pop<num_t>();
	attack  = vm->stack->Pop<num_t>();

	if (didNoteOn)
	{
		env.attack = attack;
		env.decay = decay;
		env.sustain = sustain;

		env.NoteOn();
		didNoteOn = false;
	}
	if (didNoteOff)
	{
		env.release = release;

		env.NoteOff();
		didNoteOff = false;
	}

	double val = env.Advance(kBlockSize*kDefaultOversampling);

	vm->stack->Push(val);
}

void ZADSRMod::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;

	didNoteOn = true;

	filter.Reset();
}

void ZADSRMod::NoteOff(uint32_t deltaSamples)
{
	deltaSamples;

	didNoteOff = true;
}

} // namespace invader