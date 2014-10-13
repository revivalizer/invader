#pragma once

namespace invader {

class ZWave : public align16
{
public:
	ZWave(const uint32_t size, const uint32_t reps)
		: iSize(size)
		, dSize(size)
		, reps(reps)
	{
		data = new int16_t[(size+32)*2]; // allocate 16 samples at either end, stereo
		paddedData = data+16*2;
	}

	~ZWave()
	{
		if (data)
		{
			delete[] data;
			data = nullptr;
		}
	}

	uint32_t iSize;
	double dSize;
	double reps; // number of repetitions of the waveform

	int16_t*  data;
	int16_t*  paddedData;

	void GeneratePadding()
	{
		for (int32_t i=-32; i<0; i++)
			paddedData[i] = paddedData[ i+iSize*2];

		for (int32_t i=0; i<32; i++)
			paddedData[i+iSize*2] = paddedData[i];
	}
	
private:
};

} // namespace invader