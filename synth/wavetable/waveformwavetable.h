#pragma once

namespace invader {

template <uint32_t size = 2048, uint32_t padFactor=1> // size is number of samples in table, padFacotr applied independently
class ZWaveformWavetable : public ZWavetable<size, padFactor>
{
public:
	ZWaveformWavetable(const ZRealSpectrum& spectrum, uint32_t randomSeed, double phaseVariance, double amplitudeVariance)
		: spectrum(spectrum)
		, randomSeed(randomSeed)
		, phaseVariance(phaseVariance)
		, amplitudeVariance(amplitudeVariance)
	{
	}
	
private:
	void GenerateShiftedPerturbedWave(const uint32_t factor, double waveform[size], ZRandom& r)
	{
		ZRealSpectrum shiftedSpectrum;

		// Create spectrum with shifted and pertubed harmonics
		for (uint32_t harmonic=0; harmonic<shiftedSpectrum.size; harmonic++)
		{
			if ((harmonic % factor)==0)
			{
				double angle = -kM_PI_2 + phaseVariance*r.NextGaussianDouble(); // -pi because it gives natural waveforms (actual saw for saw spectrum and so on)
				double amplitude = dbToGain(r.NextGaussianDouble()*amplitudeVariance);

				shiftedSpectrum.data[harmonic] = spectrum.data[harmonic/factor] * complex_t(zcosd(angle)*amplitude, zsind(angle)*amplitude); 
			}
			else
			{
				shiftedSpectrum.data[harmonic] = complex_t(0.0);
			}
		}

		// Transform to waveform
		complex::GFFT<10> transform;
		transform.realifft(shiftedSpectrum.data, waveform); // 11 -> 2^11 -> 2048
	}

	virtual ZWave<size>* Generate(const uint32_t harmonic)
	{
		// Shift factor
		uint32_t oct = (harmonic-15)/12; // this should really be 17 in order to have r in 0.5-1.0, but it aliases above 0.9, so...
		uint32_t factor = 1 << oct; // mul/spread factor for harmonics
		factor = factor*32;

		// Transform to waveform
		double waveformLeft[size];
		double waveformRight[size];

		ZRandom rLeft(randomSeed);
		ZRandom rRight(randomSeed+2901274);

		GenerateShiftedPerturbedWave(factor, waveformLeft, rLeft);
		GenerateShiftedPerturbedWave(factor, waveformRight, rRight);
		
		// Normalize wave
		double max = 0.0;
		for (uint32_t harmonic=0; harmonic<size; harmonic++)
		{
			max = zmax(max, zfabsd(waveformLeft[harmonic]));
			max = zmax(max, zfabsd(waveformRight[harmonic]));
		}

		for (uint32_t harmonic=0; harmonic<size; harmonic++)
		{
			waveformLeft[harmonic] /= max;
			waveformRight[harmonic] /= max;
		}
		
		// Convert to 16 bit wave
		auto wave = new ZWave<size>;
		wave->reps = factor;

		for (uint32_t harmonic=0; harmonic<size; harmonic++)
		{
			double clampedValLeft  = zclamp(waveformLeft[harmonic]*32767.0, -32767.0, 32767.0);
			double clampedValRight = zclamp(waveformRight[harmonic]*32767.0, -32767.0, 32767.0);
			wave->paddedData[harmonic*2+0] = int16_t(clampedValLeft);
			wave->paddedData[harmonic*2+1] = int16_t(clampedValRight);
		}

		return wave;
	}

	ZRealSpectrum spectrum;
	uint32_t randomSeed;
	double phaseVariance;
	double amplitudeVariance;
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