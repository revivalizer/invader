#pragma once

class ZDelay : public ZSynthNode
{
public:
	ZDelay(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);

	ZCircularBuffer<65536*8> buffer;
};

