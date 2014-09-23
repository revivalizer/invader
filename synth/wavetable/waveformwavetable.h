#pragma once

namespace invader {

template <uint32_t size = 2048, uint32_t padFactor=1> // size is number of samples in table, padFacotr applied independently
class ZWaveformWavetable : public ZWavetable<size, padFactor>
{
public:
	ZWaveformWavetable(const ZRealSpectrum& spectrum)
		: spectrum(spectrum)
	{
	}
	
private:
	virtual ZWave<size>* Generate(const uint32_t i)
	{
		uint32_t oct = i/12;
		//uint32_t shift = (2^oct) >> 1; // 0, 1, 2, 4, 8, ...
		uint32_t shift = 1 << (oct) >> 1; // 0, 1, 2, 4, 8, ...
		uint32_t repetitions = 1 << oct; // number of times the wave repreats

		ZRealSpectrum shiftedSpectrum;

		for (uint32_t i=0; i<shift; i++)
			shiftedSpectrum.data[i] = complex_t(0.0);

		for (uint32_t i=shift; i<shiftedSpectrum.size; i++)
			shiftedSpectrum.data[i] = spectrum.data[i-shift] * complex_t(0.0, -1.0); // rotate to create sin spec, rather than cos spec. not sure why this has to be -1, thought it would be +1

		double waveform[2048];

		//TOO MUCH 11?
		complex::GFFT<10> transform;
		transform.realifft(shiftedSpectrum.data, waveform); // 11 -> 2^11 -> 2048
		
		auto wave = new ZWave<size>;
		wave->reps = repetitions;

		for (uint32_t i=0; i<2048; i++)
		{
			double clampedVal = zclamp(waveform[i]*32767.0, -32767.0, 32767.0);
			wave->paddedData[i*2+0] = int16_t(clampedVal);
			wave->paddedData[i*2+1] = int16_t(clampedVal);
		}

		return wave;
	}

	ZRealSpectrum spectrum;
};

/*class ZWaveformWavetable
{
public:
	ZWaveformWavetable(const ZRealSpectrum& spectrum);
	~ZWaveformWavetable();
	
	virtual ZWave* GetWave(double pitch);

private:
	ZRealSpectrum spectrum;
	ZWave* table[11]; // 11*12 = 132 > 128
};*/

} // namespace invader