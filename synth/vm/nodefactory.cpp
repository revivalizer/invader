#include "pch.h"

#include "../nodes/adsrgain.h"
#include "../nodes/wavetableosc.h"
#include "../nodes/filter.h"

namespace invader {

ZNode* ZVirtualMachine::CreateNodeFromOpcode(nodetype_t type)
{
	switch (type)
	{
		case kOpNodeWavetableOsc:
			return new ZWavetableOsc(type);
		case kOpNodeADSR:
			return new ZADSRGain(type);
		case kOpNodeFilter1:
			return new ZFilter(type);
		default:
			MessageBoxA(nullptr, "Unhandled Case in CreateNodeFromOpcode", "Fatal Error", 0);
//			ExitProcess(9);
			return nullptr;
			break;
	}
}

} // namespace invader