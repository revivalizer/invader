#pragma once

class ZFilter : public ZSynthNode
{
public:
	ZFilter(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples);

	ZFilterZD24 filter;
};

