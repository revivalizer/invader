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
numTaps;
		for (uint32_t i=0; i<filterSize*oversampling/2; i++)
		{
			double w = double(i)/double(numTaps*oversampling);

			diff = re(spectrum[i]);

			/*if (i==0)
			{
				spectrum[i] = complex_t(0.0);
				diff = 0.0;
			}
			else*/ if (w < passband)
			{
				//diff = re(spectrum[i]) - 1.0;

				spectrum[i] = complex_t(zclamp(re(spectrum[i]), 1.0/maxErrorPass, maxErrorPass));

				/*if (re(spectrum[i]) > maxErrorPass)
					spectrum[i] = complex_t(zsqrtd(maxErrorPass));
				else if (re(spectrum[i]) < 1.0/maxErrorPass)
					spectrum[i] = complex_t(1.0/zsqrtd(maxErrorPass));
					*/


				diff -= re(spectrum[i]);
			}
			else if (w > stopband)
			{
				//diff = re(spectrum[i]);

				spectrum[i] = complex_t(zclamp(re(spectrum[i]), -maxErrorStop, maxErrorStop));


				/*
				if (re(spectrum[i]) > maxErrorStop)
					spectrum[i] = complex_t(zsqrtd(maxErrorStop));
				else if (re(spectrum[i]) < -maxErrorStop)
					spectrum[i] = complex_t(-zsqrtd(maxErrorStop));
					*/


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

	static const uint32_t filterSize = 512; // number of taps in filter, must be power of two
	static const uint32_t oversampling = 16;
	static const uint32_t n = filterSize * oversampling;

	complex_t oversampledSpectrum[n/2];
	double    oversampledImpulse[n];
	double    impulse2[numTaps + 2];


	void Init()
	{
		double passBandEnd   = 0.90 * 0.5 / numPhases;
		double stopBandStart = 1.55 * 0.5 / numPhases;

		double* impulse = impulse2+1;

		// Reset spectrum
		for (uint32_t i=0; i<n/2; i++)
			oversampledSpectrum[i] = complex_t(0.0);

		// Constrain spectrum with zero tolerance, this will generate initial guess
		ConstrainSpectrum(oversampledSpectrum, filterSize, numTaps, oversampling, passBandEnd, stopBandStart, 1.0, 0.0);
		
		complex::GFFT<12> transform;

		// Iterate -- this is guarenteed to converge, if possible
		for (uint32_t i=0; i<800; i++)
		{
			// Transform to impulse, and truncate values
			transform.realifft(oversampledSpectrum, oversampledImpulse);
			TruncateImpulse(oversampledImpulse, filterSize, numTaps, oversampling);

			// Transform to spectrum and constrain
			transform.realfft(oversampledImpulse, oversampledSpectrum);

			if (i==49 && false)
			{
				_zmsg("spec = { ");

				for (uint32_t i=0; i<n/2; i++)
					_zmsg("%f, ", re(oversampledSpectrum[i]));

				_zmsg("};");
			}

			//double error = ConstrainSpectrum(oversampledSpectrum, filterSize, numTaps, oversampling, passBandEnd, stopBandStart, 1.0 + (dbToGain(0.08) - 1.0)*zpowd(2.0, 2.0-double(i)/12.0), dbToGain(-85.0)*zpowd(2.0, 2.0-double(i)/12.0));
			double error = ConstrainSpectrum(oversampledSpectrum, filterSize, numTaps, oversampling, passBandEnd, stopBandStart, dbToGain(0.08), dbToGain(-85.0));

			error;
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

		impulse2[numTaps+1] = 0.0; // for interpolation of last tap
		impulse2[0] = 0.0; // for interpolation of last tap

		// Normalize phases individually
		for (uint32_t i=0; i<numPhases; i++)
		{
			double sum = 0.0;

			for (uint32_t j=0; j<numTapsPerPhase; j++)
				sum += impulse[i + j*numPhases];
			//_zmsg("%f", sum);

			//sum = 128.0;

			for (uint32_t j=0; j<numTapsPerPhase; j++)
				impulse[i + j*numPhases] /= sum;
		}

		// Normalize impulse
		/*double sum = 0.0;

		for (uint32_t i=0; i<numTaps; i++)
			sum += impulse[i];

		sum /= numPhases;

		for (uint32_t i=0; i<numTaps; i++)
			impulse[i] /= sum;
			*/

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
				//uint32_t rj = j; // reverse of j index, because impulse must be applied backwards

				interleavedTaps[i][j][0] = impulse[rj*numPhases + i];
				//interleavedTaps[i][j][1] = 1.0;
				interleavedTaps[i][j][1] = impulse[rj*numPhases + i + 1] - impulse[rj*numPhases + i];
			}
		}

		// Normalize phases individually
		/*for (uint32_t i=0; i<numPhases; i++)
		{
			double sum = 0.0;

			for (uint32_t j=0; j<numTapsPerPhase; j++)
				sum += interleavedTaps[i][j][0];
			//_zmsg("%f", sum);

			sum = 128.0;

			for (uint32_t j=0; j<numTapsPerPhase; j++)
				interleavedTaps[i][j][0] /= sum;
		}
		*/

		// Compute deltas to next phase and store in index 1
		/*for (uint32_t i=0; i<numPhases; i++)
		{
			for (uint32_t j=0; j<numTapsPerPhase; j++)
			{
				//interleavedTaps[i][j][1] = interleavedTaps[(i + 1) % numPhases][j][0] - interleavedTaps[i][j][0];
				interleavedTaps[i][j][1] = interleavedTaps[(i + 1) % numPhases][j][0] - interleavedTaps[i][j][0];
			}
		}*/
	}
	
private:
};

extern ZFIRInterpolator firInterpolator;

} // namespace invader
