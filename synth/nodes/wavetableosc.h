#pragma once

namespace invader { 

class ZWavetableOsc : public ZNode
{
public:
	ZWavetableOsc(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm);

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	virtual void NoteOff(uint32_t deltaSamples);

	bool handleNoteOn;

	double phase;
};

} // namespace invader
