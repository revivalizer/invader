#pragma once

namespace invader {

class ZVMStack
{
public:

	ZVMStack(uintptr_t mem) : ptr(mem)
	{
	}

	~ZVMStack(void)
	{
	}

	uintptr_t ptr;

	void PushNum(num_t num)
	{
		ptr += sizeof(num_t);
		*((double*)ptr) = num;
	}

	num_t PopNum()
	{
		ptr -= sizeof(num_t);
		return ((double*)ptr)[1];
	}

	void PushSampleBlock(ZBlockBufferInternal& block)
	{
		ptr += sizeof(ZBlockBufferInternal);
		*((ZBlockBufferInternal*)ptr) = block;
	}

	ZBlockBufferInternal& PopSampleBlock()
	{
		ptr -= sizeof(ZBlockBufferInternal);
		return ((ZBlockBufferInternal*)ptr)[1];
	}
};

} // namespace invader
