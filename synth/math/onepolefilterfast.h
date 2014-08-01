#pragma once

// Based on: http://www.earlevel.com/main/2012/12/15/a-one-pole-filter/

namespace invader {

class ZOnepoleFilterFast
{
public:
	ZINLINE ZOnepoleFilterFast(void)
	{
		Reset();
	}

	ZINLINE void Reset()
	{
		z1 = sample_t::zero();
	}

	ZINLINE sample_t Lowpass(const sample_t& input)
	{
		return z1 = input * a0 + z1 * b1;
	}

	ZINLINE sample_t Highpass(const sample_t& input)
	{
		return input-Lowpass(input);
	}

	ZINLINE void SetCutoff(double cutoff)
	{
		b1 = sample_t(zexpd(-2.0 * kM_PI * cutoff/kSampleRate));
		a0 = sample_t(1.0 - b1);
	}

	sample_t a0, b1, z1;
};

} // namespace invader
