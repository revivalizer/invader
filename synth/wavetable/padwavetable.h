#pragma once

namespace invader {

enum 
{
	kProfileGauss = 0,
	kProfileBox,
	kProfileInvGauss,
	kProfilePure,
};

enum {
	kModeNatural = 0,
	kModeMetallic,
};

bool LimitHarmonics(ZPadSpectrum& spectrum, double low, double high, uint32_t& lowOut, uint32_t& highOut);

// Profile functions
void AddGaussian(ZPadSpectrum& spectrum, double gain, double mean, double stdDev);
void AddBox(ZPadSpectrum& spectrum, double gain, double center, double radius);
void AddInvGaussian(ZPadSpectrum& spectrum, double gain, double mean, double stdDev);
void AddPure(ZPadSpectrum& spectrum, double gain, double mean, double stdDev);
void AddHarmonic(ZPadSpectrum& spectrum, uint32_t type, double gain, double mean, double stdDev);
double noise(double x, double y);

template <uint32_t size = 2048*32, uint32_t padFactor=1> // size is number of samples in table, padFacotr applied independently
class ZPadWavetable : public ZWavetable
{
public:
	ZPadWavetable(const ZRealSpectrum& spectrum, uint32_t randomSeed, uint32_t pitchScaleMode, uint32_t profile, double harmonicBandwith, double harmonicBandwithScale, double harmonicBandwithPitchScale, double detune, double detuneScale, double detuneInharmonicity)
		: spectrum(spectrum)
		, randomSeed(randomSeed)
		, pitchScaleMode(pitchScaleMode)
		, profile(profile)
		, harmonicBandwith(harmonicBandwith)
		, harmonicBandwithScale(harmonicBandwithScale)
		, harmonicBandwithPitchScale(harmonicBandwithPitchScale)
		, detune(detune)
		, detuneScale(detuneScale)
		, detuneInharmonicity(detuneInharmonicity)
	{
	}
	
private:
	void GenerateShiftedPerturbedWave(const uint32_t factor, const ZPadSpectrum& amplitudeSpectrum, double waveform[size], double randomSeed)
	{
		ZPadSpectrum* randomPhaseSpectrum = new ZPadSpectrum;
		randomPhaseSpectrum->Reset();

		// Create spectrum with shifted and pertubed harmonics
		for (uint32_t harmonic=0; harmonic<randomPhaseSpectrum->size; harmonic++)
		{
			double angle = noise(double(harmonic)/double(factor), randomSeed) * kM_PI2;
			double amplitude = 1.0;

			randomPhaseSpectrum->data[harmonic] = amplitudeSpectrum.data[harmonic] * complex_t(zcosd(angle)*amplitude, zsind(angle)*amplitude); 
		}

		// Transform to waveform
		complex::GFFT<10+5> transform;
		transform.realifft(randomPhaseSpectrum->data, waveform); // 11 -> 2^11 -> 2048

		delete randomPhaseSpectrum;
	}

	virtual ZWave* Generate(const uint32_t pitch)
	{
		// Shift factor
		uint32_t oct = (pitch-15)/12; // this should really be 17 in order to have r in 0.5-1.0, but it aliases above 0.9, so...
		uint32_t factor = 1 << oct; // mul/spread factor for harmonics
		factor = factor*32;

		// Generate amplitudes
		ZPadSpectrum* amplitudeSpectrum = new ZPadSpectrum;
		amplitudeSpectrum->Reset();

		ZRandom detuneRandom(randomSeed + 828393);

		double pitchScale = zpowd(1.0 + harmonicBandwithPitchScale, ((double)pitch - 64.0) / 12.0); // Exponentialize around mid value (64 out of 128)   (exponentializing around C-4 (48) lets it really grow out of control at higher octaves -- the exponentiation is asymmetrical)
		pitchScale /= zpowd(1.0 + harmonicBandwithPitchScale, (48.0-64.0) / 12.0); // Normalize so pitch 48 = 1, letting us control the scale around C-4

		for (uint32_t i=0; i<spectrum.size; i++)
		{
			double amplitude = magnitude(spectrum.data[i]);

			if (amplitude > 0.0)
			{
				double relativeFreq = 1.0 + zlog2d((double)(i+1));

				double pitchScaleBase = relativeFreq;
				if (pitchScaleMode==kModeNatural)
					pitchScaleBase = (double)(i+1); // this is correct according to padsynth algorithm, but keep above for backwards compatability

				double detuneOffset = 1.0 + detune * zpowd(relativeFreq, detuneScale) * detuneRandom.NextGaussianDouble();
				detuneOffset *= 1.0 + (relativeFreq - 1.0) * detuneInharmonicity;

				double partial = (double)((i+1)*factor) * detuneOffset; // We subtract 1 because we do not include a value for DC in the spectrum (so index 0 -> partial 1)

				double bandwidth =		0.5												// Base bandwidth to ensure that gaussian sampling doesn't alias to badly (especially with harmonicity)
									+	harmonicBandwith/pitchToFrequency(pitch)		// Relative bandwidth
									*	(double)factor									// Absolute bandwidth
									*	zpowd(pitchScaleBase, harmonicBandwithScale)	// Harmonic scaling
									*	pitchScale										// Pitch scaling
									/	zpowd(2.0, (double)(pitch % 12) / 12.0f);		// Correct for playback speed, which scales spectrum

				AddHarmonic(*amplitudeSpectrum, profile, amplitude, partial, bandwidth);
			}
		}

		// Transform to waveform
		double* waveformLeft = new double[size];
		double* waveformRight = new double[size];

		ZRandom rLeft(randomSeed);
		ZRandom rRight(randomSeed+2901274);

		GenerateShiftedPerturbedWave(factor, *amplitudeSpectrum, waveformLeft, double(randomSeed));
		GenerateShiftedPerturbedWave(factor, *amplitudeSpectrum, waveformRight, double(randomSeed) + 3985.08);
		
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
		auto wave = new ZWave(size, factor);

		for (uint32_t harmonic=0; harmonic<size; harmonic++)
		{
			double clampedValLeft  = zclamp(waveformLeft[harmonic]*32767.0, -32767.0, 32767.0);
			double clampedValRight = zclamp(waveformRight[harmonic]*32767.0, -32767.0, 32767.0);
			wave->paddedData[harmonic*2+0] = int16_t(clampedValLeft);
			wave->paddedData[harmonic*2+1] = int16_t(clampedValRight);
		}

		delete amplitudeSpectrum; 
		delete[] waveformLeft;
		delete[] waveformRight;

		return wave;
	}

	ZRealSpectrum spectrum;

	uint32_t randomSeed;
	uint32_t pitchScaleMode;
	uint32_t profile;

	double harmonicBandwith;
	double harmonicBandwithScale;
	double harmonicBandwithPitchScale;

	double detune;
	double detuneScale;
	double detuneInharmonicity;

	// ?
	//int maxHarmonics;
};

} // namespace invader