#pragma once

namespace invader {

template <uint32_t size, uint32_t padFactor> // size is number of samples in table, padFacotr applied independently
class ZWavetable : public align16
{
public:
	static const uint32_t kNumWaves = 128;

	ZWavetable()
	{
		for (uint32_t i=0; i<kNumWaves; i++)
			wave[i] = nullptr;
	}

	virtual ~ZWavetable()
	{
		for (uint32_t i=0; i<kNumWaves; i++)
		{
			if (wave[i])
				delete wave[i];
		}
	}

	ZWave<size>* GetWave(double pitch)
	{
		uint32_t i = zifloord(pitch);

		if (!wave[i])
			wave[i] = Generate(i);

		return wave[i];
	}

private:
	virtual ZWave<size>* Generate(uint32_t i) = 0;
	
	ZWave<size>* wave[kNumWaves];
};

} // namespace invader
