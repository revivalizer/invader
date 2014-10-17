#pragma once

namespace invader {

static const double precalcedImpulse[] = {0.000000, -0.002665, -0.000982, -0.000645, -0.000311, 0.000075, 0.000506, 0.001011, 0.001571, 0.002212, 0.002912, 0.003695, 0.004538, 0.005461, 0.006440, 0.007493, 0.008594, 0.009760, 0.010962, 0.012214, 0.013487, 0.014791, 0.016098, 0.017413, 0.018708, 0.019988, 0.021220, 0.022411, 0.023527, 0.024572, 0.025514, 0.026358, 0.027071, 0.027656, 0.028086, 0.028363, 0.028460, 0.028384, 0.028109, 0.027644, 0.026966, 0.026087, 0.024989, 0.023685, 0.022161, 0.020436, 0.018496, 0.016367, 0.014037, 0.011537, 0.008857, 0.006032, 0.003057, -0.000029, -0.003232, -0.006507, -0.009857, -0.013235, -0.016644, -0.020031, -0.023402, -0.026700, -0.029932, -0.033085, -0.035972, -0.038872, -0.041436, -0.043821, -0.045985, -0.047866, -0.049477, -0.050760, -0.051731, -0.052335, -0.052593, -0.052452, -0.051940, -0.051008, -0.049690, -0.047945, -0.045810, -0.043252, -0.040315, -0.036974, -0.033279, -0.029213, -0.024831, -0.020127, -0.015159, -0.009930, -0.004503, 0.001111, 0.006848, 0.012689, 0.018568, 0.024459, 0.030299, 0.036056, 0.041669, 0.047104, 0.052303, 0.057232, 0.061838, 0.066087, 0.069934, 0.073348, 0.076292, 0.078739, 0.080661, 0.082037, 0.082847, 0.083081, 0.082726, 0.081779, 0.080238, 0.078109, 0.075398, 0.072120, 0.068289, 0.063929, 0.059062, 0.053720, 0.047930, 0.041734, 0.035161, 0.028265, 0.021075, 0.013669, 0.006012, -0.001744, -0.009605, -0.017491, -0.025375, -0.033174, -0.040865, -0.048361, -0.055642, -0.062620, -0.069280, -0.075531, -0.081367, -0.086699, -0.091529, -0.095769, -0.099432, -0.102435, -0.104799, -0.106452, -0.107425, -0.107651, -0.107176, -0.105944, -0.104010, -0.101330, -0.097969, -0.093896, -0.089186, -0.083820, -0.077881, -0.071362, -0.064352, -0.056857, -0.048969, -0.040705, -0.032158, -0.023354, -0.014386, -0.005287, 0.003854, 0.012999, 0.022063, 0.031005, 0.039749, 0.048252, 0.056447, 0.064292, 0.071729, 0.078719, 0.085218, 0.091187, 0.096595, 0.101408, 0.105606, 0.109162, 0.112065, 0.114296, 0.115854, 0.116727, 0.116924, 0.116439, 0.115293, 0.113642, 0.110863, 0.108048, 0.104158, 0.099833, 0.094956, 0.089546, 0.083637, 0.077260, 0.070449, 0.063245, 0.055682, 0.047807, 0.039655, 0.031279, 0.022711, 0.014013, 0.005212, -0.003624, -0.012472, -0.021261, -0.029972, -0.038529, -0.046922, -0.055071, -0.062973, -0.070547, -0.077798, -0.084644, -0.091098, -0.097081, -0.102612, -0.107614, -0.112116, -0.116043, -0.119432, -0.122211, -0.124426, -0.126009, -0.127011, -0.127370, -0.127144, -0.126275, -0.124827, -0.122747, -0.120106, -0.116853, -0.113049, -0.108675, -0.103779, -0.098347, -0.092438, -0.086024, -0.079182, -0.071890, -0.064213, -0.056137, -0.047731, -0.038989, -0.029974, -0.020683, -0.011178, -0.001467, 0.008394, 0.018416, 0.028424, 0.038640, 0.048703, 0.058788, 0.068796, 0.078686, 0.088410, 0.097922, 0.107177, 0.116120, 0.124711, 0.132889, 0.140616, 0.147827, 0.154489, 0.160531, 0.165925, 0.170602, 0.174537, 0.177664, 0.179963, 0.181370, 0.181877, 0.181424, 0.180011, 0.177588, 0.174162, 0.169697, 0.164209, 0.157673, 0.150118, 0.141535, 0.131958, 0.121399, 0.109901, 0.097492, 0.084224, 0.070143, 0.055307, 0.039781, 0.023627, 0.006932, -0.010247, -0.027801, -0.045675, -0.063743, -0.081950, -0.100161, -0.118324, -0.136288, -0.154013, -0.171335, -0.188222, -0.204504, -0.220157, -0.235003, -0.249034, -0.262066, -0.274102, -0.284957, -0.294646, -0.302988, -0.310018, -0.315985, -0.319115, -0.322310, -0.322517, -0.321399, -0.318574, -0.313873, -0.307354, -0.298873, -0.288500, -0.276110, -0.261783, -0.245411, -0.227086, -0.206725, -0.184427, -0.160136, -0.133956, -0.105865, -0.075967, -0.044277, -0.010899, 0.024119, 0.060671, 0.098675, 0.138030, 0.178615, 0.220340, 0.263048, 0.306659, 0.350984, 0.395957, 0.441358, 0.487141, 0.533056, 0.579081, 0.624941, 0.670639, 0.715878, 0.760691, 0.804762, 0.848156, 0.890539, 0.932014, 0.972233, 1.011334, 1.048957, 1.085263, 1.119910, 1.153077, 1.184414, 1.214145, 1.241897, 1.267939, 1.291897, 1.314056, 1.334043, 1.352172, 1.368074, 1.382078, 1.393815, 1.403635, 1.411185, 1.416866, 1.422247, 1.419575, 1.422247, 1.416866, 1.411185, 1.403635, 1.393815, 1.382078, 1.368074, 1.352172, 1.334043, 1.314056, 1.291897, 1.267939, 1.241897, 1.214145, 1.184414, 1.153077, 1.119910, 1.085263, 1.048957, 1.011334, 0.972233, 0.932014, 0.890539, 0.848156, 0.804762, 0.760691, 0.715878, 0.670639, 0.624941, 0.579081, 0.533056, 0.487141, 0.441358, 0.395957, 0.350984, 0.306659, 0.263048, 0.220340, 0.178615, 0.138030, 0.098675, 0.060671, 0.024119, -0.010899, -0.044277, -0.075967, -0.105865, -0.133956, -0.160136, -0.184427, -0.206725, -0.227086, -0.245411, -0.261783, -0.276110, -0.288500, -0.298873, -0.307354, -0.313873, -0.318574, -0.321399, -0.322517, -0.322310, -0.319115, -0.315985, -0.310018, -0.302988, -0.294646, -0.284957, -0.274102, -0.262066, -0.249034, -0.235003, -0.220157, -0.204504, -0.188222, -0.171335, -0.154013, -0.136288, -0.118324, -0.100161, -0.081950, -0.063743, -0.045675, -0.027801, -0.010247, 0.006932, 0.023627, 0.039781, 0.055307, 0.070143, 0.084224, 0.097492, 0.109901, 0.121399, 0.131958, 0.141535, 0.150118, 0.157673, 0.164209, 0.169697, 0.174162, 0.177588, 0.180011, 0.181424, 0.181877, 0.181370, 0.179963, 0.177664, 0.174537, 0.170602, 0.165925, 0.160531, 0.154489, 0.147827, 0.140616, 0.132889, 0.124711, 0.116120, 0.107177, 0.097922, 0.088410, 0.078686, 0.068796, 0.058788, 0.048703, 0.038640, 0.028424, 0.018416, 0.008394, -0.001467, -0.011178, -0.020683, -0.029974, -0.038989, -0.047731, -0.056137, -0.064213, -0.071890, -0.079182, -0.086024, -0.092438, -0.098347, -0.103779, -0.108675, -0.113049, -0.116853, -0.120106, -0.122747, -0.124827, -0.126275, -0.127144, -0.127370, -0.127011, -0.126009, -0.124426, -0.122211, -0.119432, -0.116043, -0.112116, -0.107614, -0.102612, -0.097081, -0.091098, -0.084644, -0.077798, -0.070547, -0.062973, -0.055071, -0.046922, -0.038529, -0.029972, -0.021261, -0.012472, -0.003624, 0.005212, 0.014013, 0.022711, 0.031279, 0.039655, 0.047807, 0.055682, 0.063245, 0.070449, 0.077260, 0.083637, 0.089546, 0.094956, 0.099833, 0.104158, 0.108048, 0.110863, 0.113642, 0.115293, 0.116439, 0.116924, 0.116727, 0.115854, 0.114296, 0.112065, 0.109162, 0.105606, 0.101408, 0.096595, 0.091187, 0.085218, 0.078719, 0.071729, 0.064292, 0.056447, 0.048252, 0.039749, 0.031005, 0.022063, 0.012999, 0.003854, -0.005287, -0.014386, -0.023354, -0.032158, -0.040705, -0.048969, -0.056857, -0.064352, -0.071362, -0.077881, -0.083820, -0.089186, -0.093896, -0.097969, -0.101330, -0.104010, -0.105944, -0.107176, -0.107651, -0.107425, -0.106452, -0.104799, -0.102435, -0.099432, -0.095769, -0.091529, -0.086699, -0.081367, -0.075531, -0.069280, -0.062620, -0.055642, -0.048361, -0.040865, -0.033174, -0.025375, -0.017491, -0.009605, -0.001744, 0.006012, 0.013669, 0.021075, 0.028265, 0.035161, 0.041734, 0.047930, 0.053720, 0.059062, 0.063929, 0.068289, 0.072120, 0.075398, 0.078109, 0.080238, 0.081779, 0.082726, 0.083081, 0.082847, 0.082037, 0.080661, 0.078739, 0.076292, 0.073348, 0.069934, 0.066087, 0.061838, 0.057232, 0.052303, 0.047104, 0.041669, 0.036056, 0.030299, 0.024459, 0.018568, 0.012689, 0.006848, 0.001111, -0.004503, -0.009930, -0.015159, -0.020127, -0.024831, -0.029213, -0.033279, -0.036974, -0.040315, -0.043252, -0.045810, -0.047945, -0.049690, -0.051008, -0.051940, -0.052452, -0.052593, -0.052335, -0.051731, -0.050760, -0.049477, -0.047866, -0.045985, -0.043821, -0.041436, -0.038872, -0.035972, -0.033085, -0.029932, -0.026700, -0.023402, -0.020031, -0.016644, -0.013235, -0.009857, -0.006507, -0.003232, -0.000029, 0.003057, 0.006032, 0.008857, 0.011537, 0.014037, 0.016367, 0.018496, 0.020436, 0.022161, 0.023685, 0.024989, 0.026087, 0.026966, 0.027644, 0.028109, 0.028384, 0.028460, 0.028363, 0.028086, 0.027656, 0.027071, 0.026358, 0.025514, 0.024572, 0.023527, 0.022411, 0.021220, 0.019988, 0.018708, 0.017413, 0.016098, 0.014791, 0.013487, 0.012214, 0.010962, 0.009760, 0.008594, 0.007493, 0.006440, 0.005461, 0.004538, 0.003695, 0.002912, 0.002212, 0.001571, 0.001011, 0.000506, 0.000075, -0.000311, -0.000645, -0.000982, -0.002665, 0.000000, };

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
	double    impulse[numTaps + 1];


	void Init()
	{
		#if 0 // generate FIR impulse or use precalc?
			double passBandEnd   = 0.90 * 0.5 / numPhases;
			double stopBandStart = 1.55 * 0.5 / numPhases;

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
				//_zmsg("%f", error);
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

			// Normalize phases individually
			for (uint32_t i=0; i<numPhases; i++)
			{
				double sum = 0.0;

				for (uint32_t j=0; j<numTapsPerPhase; j++)
					sum += impulse[i + j*numPhases];

				for (uint32_t j=0; j<numTapsPerPhase; j++)
					impulse[i + j*numPhases] /= sum;
			}

			// Zero pad
			impulse[numTaps] = 0.0; // for interpolation of last tap

			// Print impulse response
			for (uint32_t i=0; i<numTaps+1; i++)
				_zmsg("%f, ", impulse[i]);
		#else
			// Load precalced version
			const double* impulse = precalcedImpulse;
		#endif

		// Create interleaved taps
		for (uint32_t i=0; i<numPhases; i++)
		{
			for (uint32_t j=0; j<numTapsPerPhase; j++)
			{
				uint32_t rj = numTapsPerPhase-j-1; // reverse of j index, because impulse must be applied backwards

				interleavedTaps[i][j][0] = impulse[rj*numPhases + i];
				interleavedTaps[i][j][1] = impulse[rj*numPhases + i + 1] - impulse[rj*numPhases + i];
			}
		}
	}
	
private:
};

extern ZFIRInterpolator firInterpolator;

} // namespace invader
