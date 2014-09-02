#pragma once

namespace invader {

class ZFIRInterpolator
{
public:
	//ZFIRInterpolator();
	//~ZFIRInterpolator();

	void TruncateImpulse(double* impulse, const uint32_t numTaps, const uint32_t oversampling)
	{
		for (uint32_t i=0; i<numTaps*oversampling; i++)
		{
			// keep first n/2 and last n/2-1 (n=numTaps)
			if (i>=numTaps/2 && i<=(numTaps*oversampling)-numTaps/2)
				impulse[i] = 0.0;
		}
	}

	double ConstrainSpectrum(complex_t* spectrum, const uint32_t numTaps, const uint32_t oversampling, double passband, double stopband, double maxErrorPass, double maxErrorStop)
	{
		double error = 0.0;
		double diff;

		for (uint32_t i=0; i<numTaps*oversampling/2; i++)
		{
			double w = double(i)/double(numTaps*oversampling/2);

			diff = re(spectrum[i]);

			/*if (i==0)
			{
				spectrum[i] = complex_t(0.0);
				diff = 0.0;
			}
			else*/ if (w < passband)
			{
				spectrum[i] = complex_t(zclamp(re(spectrum[i]), 1.0/maxErrorPass, maxErrorPass));
				diff -= re(spectrum[i]);
			}
			else if (w > stopband)
			{
				spectrum[i] = complex_t(zclamp(re(spectrum[i]), -maxErrorStop, maxErrorStop));
				diff -= re(spectrum[i]);
			}
			else
			{
				diff = 0.0;
			}

			error += diff*diff;
		}

		return zsqrtd(error/double(numTaps*oversampling/2));
	}

	void Init()
	{
		const uint32_t numTaps = 16;
		const uint32_t oversampling = 16;
		const uint32_t n = numTaps * oversampling;

		complex_t spectrum[n/2];
		double    impulse[n];

		for (uint32_t i=0; i<n/2; i++)
			spectrum[i] = complex_t(0.0);

		ConstrainSpectrum(spectrum, numTaps, oversampling, 0.1, 0.3, 1.0, 0.0);
		
		complex::GFFT<7> transform;

		for (uint32_t i=0; i<50; i++)
		{
			transform.realifft(spectrum, impulse);

			TruncateImpulse(impulse, numTaps, oversampling);

			transform.realfft(impulse, spectrum);

			double error = ConstrainSpectrum(spectrum, numTaps, oversampling, 0.1, 0.3, dbToGain(0.08), dbToGain(-85.0));
			_zmsg("%f", error);
		}
	}
	
private:
};

} // namespace invader
