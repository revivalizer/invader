#pragma once

namespace invader {

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
			{
				delete wave[i];
				wave[i] = nullptr;
			}
		}
	}

	ZWave* GetWave(double pitch)
	{
		uint32_t i = zitruncd(pitch);

		if (!wave[i])
		{
			wave[i] = Generate(i);
			wave[i]->GeneratePadding();
		}

		return wave[i];
	}

private:
	virtual ZWave* Generate(const uint32_t i) = 0;
	
	ZWave* wave[kNumWaves];
};

} // namespace invader
