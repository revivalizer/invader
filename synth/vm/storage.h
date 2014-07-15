#pragma once

namespace invader {

class ZVMStorage
{
public:

	ZVMStorage(void)
	{
	}

	~ZVMStorage(void)
	{
	}

	void StoreNum(opcode_index_t address, num_t num)
	{
		*((num_t*)(mem[address])) = num;
	}

	num_t LoadNum(opcode_index_t address)
	{
		return *((num_t*)(mem[address]));
	}

	void StoreSampleBlock(opcode_index_t address, ZBlockBufferInternal& block)
	{
		*((ZBlockBufferInternal*)(mem[address])) = block;
	}

	ZBlockBufferInternal& LoadSampleBlock(opcode_index_t address)
	{
		return *((ZBlockBufferInternal*)(mem[address]));
	}


	uintptr_t* mem;
};

} // namespace invader