#pragma once

namespace invader {

class ZVMStorage
{
public:

	ZVMStorage(uintptr_t mem)
		: mem(mem)
	{
	}

	~ZVMStorage(void)
	{
	}

	template <class T>
	void Store(opcode_index_t address, const T& data)
	{
		*((T*)(mem+address)) = data;
	}

	template <class T>
	T& Load(opcode_index_t address)
	{
		return *((T*)(mem+address));
	}

	template <class T>
	void Reset(opcode_index_t address)
	{
		zzeromem((void*)(mem+address), sizeof(T));
	}


	/*void StoreSampleBlock(opcode_index_t address, ZBlockBufferInternal& block)
	{
		*((ZBlockBufferInternal*)(mem[address])) = block;
	}

	ZBlockBufferInternal& LoadSampleBlock(opcode_index_t address)
	{
		return *((ZBlockBufferInternal*)(mem[address]));
	}*/

	uintptr_t mem;
};

} // namespace invader