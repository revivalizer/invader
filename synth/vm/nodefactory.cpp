#include "pch.h"

#include "../nodes/adsrgain.h"
#include "../nodes/wavetableosc.h"
#include "../nodes/filter.h"
#include "../nodes/compressor.h"
#include "../nodes/reverb.h"

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
		case kOpNodeCompress:
			return new ZCompressor(type);
		case kOpNodeSidechainCompress:
			return new ZCompressor(type);
		case kOpNodeReverb:
			return new ZReverb(type);
		default:
			MessageBoxA(nullptr, "Unhandled Case in CreateNodeFromOpcode", "Fatal Error", 0);
//			ExitProcess(9);
			return nullptr;
			break;
	}
}

} // namespace invader