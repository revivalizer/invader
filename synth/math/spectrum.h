#pragma once

namespace invader {

template <class T, uint32_t S>
class ZSpectrum : public align16
{
public:
	T data[S];
	static const uint32_t size = S;

	void Reset()
	{
		for (uint32_t i=0; i<size; i++)
			data[0] = complex_t(0.0);
	}

	void ApplyPeak(double harmonic, double width, double dB)
	{
		for (int i=1; i<size; i++)
		{
			double diff = harmonic - i;
			double normalValue = zexpd(-(diff*diff) / (2*width*width));
			data[i] *= dbToGain(normalValue * dB);
		};
	}

	void ApplyLowpass(double harmonic, double dbPerOctave)
	{
		double harmonicOctave = zlog2d(harmonic + 1.0);
		
		for (int i=zitruncd(harmonic); i<size; i++)
		{
			data[i] *= dbToGain((zlog2d(i + 1.0) - harmonicOctave)*dbPerOctave);
		};
	}

	void ApplyHighpass(double harmonic, double dbPerOctave)
	{
		double harmonicOctave = zlog2d(harmonic + 1.0);
		
		for (int i=1; i<ziceild(harmonic); i++)
		{
			data[i] *= dbToGain((harmonicOctave - zlog2d(i + 1.0))*dbPerOctave);
		};
	}
};

typedef ZSpectrum<complex_t, 256> ZRealSpectrum;
typedef ZSpectrum<complex_t, 2048*32> ZPadSpectrum;

} // namespace invader
