#pragma once

namespace invader {

class ZFIRInterpolator
{
public:
	ZFIRInterpolator() {}
	//~ZFIRInterpolator();

	static const uint32_t numTapsPerPhase = 12;
	static const uint32_t numPhases = 64;
	static const uint32_t numTaps = numTapsPerPhase * numPhases; // (=768) number of taps desired in filter, not neccesarily power of two`

	double interleavedTaps[numPhases][numTapsPerPhase][2]; // last index: value, delta

	void TruncateImpulse(double* impulse, const uint32_t filterSize, const uint32_t numTaps, const uint32_t oversampling)
	{
		for (uint32_t i=0; i<filterSize*oversampling; i++)
		{
			// keep first n/2 and last n/2-1 (n=numTaps)
			if (i>=numTaps/2 && i<=(filterSize*oversampling)-numTaps/2)
				impulse[i] = 0.0;
		}
	}

	double ConstrainSpectrum(complex_t* spectrum, const uint32_t filterSize, const uint32_t numTaps, const uint32_t oversampling, double passband, double stopband, double maxErrorPass, double maxErrorStop)
	{
		double error = 0.0;
		double diff;

		for (uint32_t i=0; i<filterSize*oversampling/2; i++)
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

		return zsqrtd(error/double(filterSize*oversampling/2));
	}

	static const uint32_t filterSize = 1024; // number of taps in filter, must be power of two
	static const uint32_t oversampling = 16;
	static const uint32_t n = filterSize * oversampling;

	complex_t oversampledSpectrum[n/2];
	double    oversampledImpulse[n];
	double    impulse[numTaps];


	void Init()
	{
		// Reset spectrum
		for (uint32_t i=0; i<n/2; i++)
			oversampledSpectrum[i] = complex_t(0.0);

		// Constrain spectrum with zero tolerance, this will generate initial guess
		ConstrainSpectrum(oversampledSpectrum, filterSize, numTaps, oversampling, 0.1, 0.3, 1.0, 0.0);
		
		complex::GFFT<13> transform;

		// Iterate -- this is guarenteed to converge, if possible
		for (uint32_t i=0; i<50; i++)
		{
			// Transform to impulse, and truncate values
			transform.realifft(oversampledSpectrum, oversampledImpulse);
			TruncateImpulse(oversampledImpulse, filterSize, numTaps, oversampling);

			// Transform to spectrum and constrain
			transform.realfft(oversampledImpulse, oversampledSpectrum);
			double error = ConstrainSpectrum(oversampledSpectrum, filterSize, numTaps, oversampling, 0.1, 0.3, dbToGain(0.08), dbToGain(-85.0));

			// Output error
			_zmsg("%f", error);
		}

		// Get impulse (currently centered on i==0)
		for (uint32_t i=0; i<numTaps; i++)
		{
			int32_t j = i - numTaps/2;

			// wrap index to end
			if (j < 0)
				j += filterSize*oversampling;

			impulse[i] = oversampledImpulse[j];
		}

		// Normalize impulse
		//double sum = 0.0;

		//for (uint32_t i=0; i<numTaps; i++)
			//sum += impulse[i];

		/*double scale = 1.0/impulse[numTaps/2]; // middle tap must be 1.0
		for (uint32_t i=0; i<numTaps; i++)
			impulse[i] *= scale;
			*/
		
		// Create interleaved taps
		for (uint32_t i=0; i<numPhases; i++)
		{
			for (uint32_t j=0; j<numTapsPerPhase; j++)
			{
				uint32_t rj = numTapsPerPhase-j-1; // reverse of j index, because impulse must be applied backwards

				interleavedTaps[i][j][0] = impulse[rj*numPhases + i];
			}
		}

		// Normalize phases individually
		for (uint32_t i=0; i<numPhases; i++)
		{
			double sum = 0.0;

			for (uint32_t j=0; j<numTapsPerPhase; j++)
				sum += interleavedTaps[i][j][0];

			for (uint32_t j=0; j<numTapsPerPhase; j++)
				interleavedTaps[i][j][0] /= sum;
		}

	}
	
private:
};

extern ZFIRInterpolator firInterpolator;

} // namespace invader
