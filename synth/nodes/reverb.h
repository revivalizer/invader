#pragma once

namespace invader {

class ZReverb : public ZNode
{
public:
	ZReverb(nodetype_t type);

	virtual void Process(ZVirtualMachine* vm);

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples) { pitch; note; velocity; deltaSamples; }
	virtual void NoteOff(uint32_t deltaSamples) { deltaSamples; }
};


} // namespace invader