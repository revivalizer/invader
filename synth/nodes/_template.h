#pragma once

namespace invader {

class ZReverb : public ZNode
{
public:
	ZReverb(nodetype_t type);

	virtual void Process(ZVirtualMachine* vm);
};


} // namespace invader