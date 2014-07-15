#pragma once

class ZADSRGain : public ZSynthNode
{
public:
	ZADSRGain(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	virtual void NoteOff(uint32_t deltaSamples);

	ZADSREnvelope env;

	double attack, decay, sustain, release;

	bool didNoteOn, didNoteOff;

	ZOnepoleFilterFast filter;
};

