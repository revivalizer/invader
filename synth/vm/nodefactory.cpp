#include "pch.h"

namespace invader {

ZNode* ZVirtualMachine::CreateNodeFromOpcode(nodetype_t type)
{
	switch (type)
	{
		case kOpNodeADSR:
			return new 	
		default:
			MessageBoxA(nullptr, "Unhandled Case in CreateNodeFromOpcode", "Fatal Error", 0);
			ExitProcess(9);
			break;
	}

	return nullptr;
}

} // namespace invader