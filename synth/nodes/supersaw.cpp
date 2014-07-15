#include "pch.h"
#include "supersaw.h"

ZSuperSaw::ZSuperSaw(nodetype_t type) : ZSynthNode(type)
	, rand(812729)
	, unisonShape(0.12)
{

}

void ZSuperSaw::Process(ZVirtualMachine* vm, argument_t argument)
{
	// Much of this from: http://www.nada.kth.se/utbildning/grukth/exjobb/rapportlistor/2010/rapporter10/szabo_adam_10131.pdf
	argument;

	static const sample_t k1 = 1.0;
	static const sample_t k2 = 2.0;

	double detunes[7] = {-0.11002313, -0.06288439, -0.01952356, 0.0, 0.01991221, 0.06216538, 0.10745242};

	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal block;

	double detune = vm->stack[-3];
	double unison = zclamp(vm->stack[-2] / 100.0, 0.0, 1.0);
	double mix    = zclamp(vm->stack[-1] / 100.0, 0.0, 1.0);
	vm->stack -= 3;

	sample_t deltaPhase[kNumOsc];
	sample_t mixSample[kNumOsc];

	double freq = pitchToFrequency(synthvm->voice->pitch + detune);
	double dPhase  = 2.0*freq/kSampleRate;

	sample_t mixCenter = 1.0 - 0.55*mix;
	sample_t mixSide   = -0.738*mix*mix + 1.28*mix + 0.0443;

	for (uint32_t i=0; i<kNumOsc; i++)
	{
		deltaPhase[i] = dPhase * (1.0 + unisonShape(unison)*detunes[i]);
		mixSample[i] = (i==kNumOsc/2) ? mixCenter : mixSide;
	}

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		block.samples[i] = sample_t::zero();

		for (uint32_t j=0; j<kNumOsc; j++)
		{
			block.samples[i] += phase[j] * mixSample[j];
			phase[j] += deltaPhase[j];
			phase[j] = select(phase[j] > k1, phase[j]-k2, phase[j]);
		}
	}

	filter.SetParameters(kFilterTypeHP24, freq, 0.3);
	filter.Process(block);

	synthvm->blockStack->Push(block);
}

sample_t GetUniformSample(ZRandom& rand)
{
	sample_t r;
	r.d[0] = (double)rand.NextUniformFloat();
	r.d[1] = (double)rand.NextUniformFloat();
	return r;
}

void ZSuperSaw::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;

	// Reset phases
	for (uint32_t i=0; i<kNumOsc; i++)
		phase[i] = GetUniformSample(rand)*2.0-1.0;

	filter.Reset();
}
