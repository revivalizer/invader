#include "pch.h"

sample_t ComputePan(double panLawdB, double pan)
{
	// Pan to gain from this discussion:
	// http://music.columbia.edu/pipermail/music-dsp/2002-September/050872.html	
	
	double scale = 2.0 - 4.0 * zpowd(10.0, panLawdB / 20.0);

	double panLeft = 1.0-pan;
	double panRight = pan;

	return sample_t(
			zclamp(scale * panLeft*panLeft + (1.0 - scale) * panLeft, 0.0, 1.0),
			zclamp(scale * panRight*panRight + (1.0 - scale) * panRight, 0.0, 1.0)
		);
}