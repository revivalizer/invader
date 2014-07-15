#pragma once


// Based on this structure: http://www.uaudio.com/blog/allpass-filters/

template<uint32_t numSamples> // numSamples MUST be power of 2
class ZDiffuser
{
public:
	ZDiffuser(void)
	{
		buffer.Reset();
	}

	sample_t Process(const sample_t& in)
	{
		sample_t out = k*in + buffer.ReadOffsetSample(size);
		buffer.PutSample(in - k*out);
		return out;
	}

	uint32_t size;
	sample_t k;

	ZCircularBuffer<numSamples> buffer;
};

