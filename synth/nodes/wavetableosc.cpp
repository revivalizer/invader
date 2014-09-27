#include "pch.h"
#include "wavetableosc.h"

namespace invader { 

ZWavetableOsc::ZWavetableOsc(nodetype_t type) : ZNode(type)
{
	handleNoteOn = false;
}

sample_t GetInterpolatedValue(const double phase, const ZWave<2048>* const wave, const ZFIRInterpolator& interpolator)
{
	uint32_t i = zifloord(phase);
	double fracPhase = phase - i;

	uint32_t interpolatorPhase = zifloord(fracPhase * interpolator.numPhases);
	double fracInterpolatorPhase = fracPhase * interpolator.numPhases - interpolatorPhase; // this must used to interpolate coeffs
	fracInterpolatorPhase;

//	SWITCH YO SYNTH INTERP

	sample_t out = sample_t::zero();

	for (uint32_t j = 0; j<interpolator.numTapsPerPhase; j++)
	{
		int32_t offset = i+j-interpolator.numTapsPerPhase/2;
		sample_t sample = sample_t(double(wave->paddedData[offset*2+0])/32768.0, double(wave->paddedData[offset*2+1])/32768.0);
		//out += sample_t(interpolator.interleavedTaps[interpolatorPhase][j][0])*sample_t(double(wave->paddedData[i+j-interpolator.numTapsPerPhase/2])/32768.0);
		out += sample_t(interpolator.interleavedTaps[interpolatorPhase][j][0] + fracInterpolatorPhase*interpolator.interleavedTaps[interpolatorPhase][j][1])*sample;
		//out += sample_t(interpolator.interleavedTaps[interpolatorPhase][j][0])*sample;
	}

	return out;
}

void ZWavetableOsc::Process(ZVirtualMachine* vm)
{
	if (handleNoteOn)
	{
		phase = 0.0;

		handleNoteOn = false;
	}

	auto wavetable = vm->stack->Pop<ZWaveformWavetable<>*>();
	wavetable;

	ZBlockBufferInternal block;

	auto wave = wavetable->GetWave(vm->voice->pitch);

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		//double time = vm->voice->timeSinceNoteOn + i/kSampleRate;
		//block.samples[zifloord(i)] = sample_t(zsind(time*pitchToFrequency(vm->voice->pitch)*kM_PI2));

		block.samples[i] = GetInterpolatedValue(phase, wave, vm->synth->firInterpolator);

		double freq = pitchToFrequency(vm->voice->pitch);
		double deltaPhase = freq / kSampleRate * wave->dSize / wave->reps;
		phase += deltaPhase;

		if (phase > wavetable->size)
			phase -= wavetable->size;
	}

	vm->stack->Push(block);
}

void ZWavetableOsc::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;

	handleNoteOn = true;
}

void ZWavetableOsc::NoteOff(uint32_t deltaSamples)
{
	deltaSamples;
}

} // namespace invader