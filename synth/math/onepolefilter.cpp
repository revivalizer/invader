#include "pch.h"

namespace invader {

template<uint32_t oversamplingFactor>
ZBlockBuffer<oversamplingFactor>& ZOnepoleFilterLowpassBlock(ZOnepoleFilter& filter, ZBlockBuffer<oversamplingFactor>& block)
{
	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i] = filter.Lowpass(block.samples[i]);
	}

	return block;
}

template<uint32_t oversamplingFactor>
ZBlockBuffer<oversamplingFactor>&  ZOnepoleFilterHighpassBlock(ZOnepoleFilter& filter, ZBlockBuffer<oversamplingFactor>& block)
{
	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i] = filter.Highpass(block.samples[i]);
	}

	return block;
}

template ZBlockBuffer<kDefaultOversampling>& ZOnepoleFilterHighpassBlock(ZOnepoleFilter& filter, ZBlockBuffer<kDefaultOversampling>& block);
template ZBlockBuffer<kDefaultOversampling>& ZOnepoleFilterLowpassBlock(ZOnepoleFilter& filter, ZBlockBuffer<kDefaultOversampling>& block);

} // namespace invader
