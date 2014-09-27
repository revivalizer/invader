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
	virtual ZWave<size>* Generate(const uint32_t harmonic)
	{
		ZRealSpectrum shiftedSpectrum;

		// Create spectrum with shifted harmonics
		uint32_t oct = (harmonic-15)/12; // this should really be 17 in order to have r in 0.5-1.0, but it aliases above 0.9, so...

		uint32_t factor = 1 << oct; // mul/spread factor for harmonics

		for (uint32_t harmonic=0; harmonic<shiftedSpectrum.size; harmonic++)
		{
			if ((harmonic % factor)==0)
				shiftedSpectrum.data[harmonic] = spectrum.data[harmonic/factor] * complex_t(0.0, -1.0); 
			else
				shiftedSpectrum.data[harmonic] = complex_t(0.0);
		}

		// Transform to waveform
		double waveform[2048];

		complex::GFFT<10> transform;
		transform.realifft(shiftedSpectrum.data, waveform); // 11 -> 2^11 -> 2048
		
		auto wave = new ZWave<size>;
		wave->reps = factor;

		// Normalize wave
		double max = 0.0;
		for (uint32_t harmonic=0; harmonic<2048; harmonic++)
			max = zmax(max, zfabsd(waveform[harmonic]));

		for (uint32_t harmonic=0; harmonic<2048; harmonic++)
			waveform[harmonic] /= max;

		// Convert to 16 bit
		for (uint32_t harmonic=0; harmonic<2048; harmonic++)
		{
			double clampedVal = zclamp(waveform[harmonic]*32767.0, -32767.0, 32767.0);
			wave->paddedData[harmonic*2+0] = int16_t(clampedVal);
			wave->paddedData[harmonic*2+1] = int16_t(clampedVal);
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