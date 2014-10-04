#pragma once

namespace invader {

class ZVirtualMachine;

class ZNode
{
public:
	ZNode(nodetype_t type);
	virtual ~ZNode();

	virtual void Process(ZVirtualMachine* vm) = 0;

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples) = 0;
	virtual void NoteOff(uint32_t deltaSamples) = 0;

	nodetype_t type;
};

} // namespace invader