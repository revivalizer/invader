#pragma once

class ZMixer : public ZSynthNode
{
public:
	ZMixer(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t numInputs);
};

