#pragma once

namespace invader {

template <uint32_t size>
class ZWave : public align16
{
public:
	ZWave()
	{
		paddedData = data+16*2;
	}

	static const uint32_t iSize = size;
	static const double   dSize;

	int16_t   data[(size+32)*2]; // allocate 16 samples at either end - stereo
	int16_t*  paddedData;

	double reps; // number of repetitions of the waveform

	void GeneratePadding()
	{
		for (int32_t i=-32; i<0; i++)
			paddedData[i] = paddedData[ i+size*2];

		for (int32_t i=0; i<32; i++)
			paddedData[i+size*2] = paddedData[i];
	}
	
private:
};

template <uint32_t size>
const double ZWave<size>::dSize = double(size);

} // namespace invader