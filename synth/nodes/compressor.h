#pragma once

namespace invader {

class ZCompressor : public ZNode
{
public:
	ZCompressor(nodetype_t type);

	virtual void Process(ZVirtualMachine* vm);

	double rmsSquared;
	double envelope;
};


} // namespace invader