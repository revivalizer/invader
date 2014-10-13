#include "pch.h"

namespace invader {

bool LimitHarmonics(ZPadSpectrum& spectrum, double low, double high, uint32_t& lowOut, uint32_t& highOut)
{
	spectrum; // compiler error, thinks spectrun is unreferenced

	if (high < 0.0 || low >= spectrum.size)
		return false;
	
	lowOut = zifloord(zclamp(low, 0.0, (double)(spectrum.size-1)));
	highOut = ziceild(zclamp(high, 0.0, (double)(spectrum.size-1)));

	return true;
}

// Profile functions
void AddGaussian(ZPadSpectrum& spectrum, double gain, double mean, double stdDev)
{
	// Find harmonic boundaries, or return if outside range
	uint32_t lowestHarmonic, highestHarmonic;
	if (!LimitHarmonics(spectrum, mean - 3.0*stdDev, mean + 3.0*stdDev, lowestHarmonic, highestHarmonic))
		return;

	double invDivisor = 1.0 / (2.0 * stdDev * stdDev);

	double multiplier = gain / stdDev;

	for (uint32_t i=lowestHarmonic; i<=highestHarmonic; i++)
	{
		double diff = (double)i-mean;
		spectrum.data[i] += complex_t(multiplier * zexpd( -diff*diff*invDivisor ));
	}
}

void AddBox(ZPadSpectrum& spectrum, double gain, double center, double radius)
{
	// Find harmonic boundaries, or return if outside range
	uint32_t lowestHarmonic, highestHarmonic;
	if (!LimitHarmonics(spectrum, center - radius, center + radius, lowestHarmonic, highestHarmonic))
		return;

	double value = gain * 1.0 / (double)(highestHarmonic - lowestHarmonic);

	for (uint32_t i=lowestHarmonic; i<=highestHarmonic; i++)
	{
		spectrum.data[i] += complex_t(value);
	}
}

void AddInvGaussian(ZPadSpectrum& spectrum, double gain, double mean, double stdDev)
{
	// Find harmonic boundaries, or return if outside range
	uint32_t lowestHarmonic, highestHarmonic;
	if (!LimitHarmonics(spectrum, mean - 3.0*stdDev, mean + 3.0*stdDev, lowestHarmonic, highestHarmonic))
		return;

	double invDivisor = 1.0 / (2.0 * stdDev * stdDev);
	double sum = 0.0;

	for (double i=(double)lowestHarmonic; i<=(double)highestHarmonic; i += 1.0)
	{
		double diff = i-mean;
		sum += 1.0 - zexpd( -diff*diff*invDivisor );
	}

	if (sum > 0.0001)
	{
		double multiplier = gain * 1.0 / sum;

		for (uint32_t i=lowestHarmonic; i<=highestHarmonic; i++)
		{
			double diff = (double)i-mean;
			spectrum.data[i] += complex_t(multiplier * (1.0 - zexpd( -diff*diff*invDivisor )));
		}
	}
}

void AddPure(ZPadSpectrum& spectrum, double gain, double mean, double stdDev)
{
	stdDev; // not used

	uint32_t harmonic = ziroundd(mean);

 	if (harmonic >= 0 && harmonic < spectrum.size)
		spectrum.data[harmonic] += complex_t(gain);
}

void AddHarmonic(ZPadSpectrum& spectrum, uint32_t type, double gain, double mean, double stdDev)
{
	switch (type)
	{
		case kProfileGauss:
			AddGaussian(spectrum, gain, mean, stdDev);
			break;
		case kProfileBox:
			AddBox(spectrum, gain, mean, stdDev);
			break;
		case kProfileInvGauss:
			AddInvGaussian(spectrum, gain, mean, stdDev);
			break;
		case kProfilePure:
			AddPure(spectrum, gain, mean, stdDev);
			break;
	}
}

double noise(double x, double y)
{
	// Based on iqs simple 2D noise
	double r = zsind(x*12.98773298 + y*78.2334125)*43758.5453;
	return r - double(zifloord(r));
}

} // namespace invader