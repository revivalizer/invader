#include "pch.h"
#include "compressor.h"

namespace invader {

ZCompressor::ZCompressor(nodetype_t type) : ZNode(type)
	, envelope(0.0)
	, rmsSquared(0.0)
{

}

void ZCompressor::Process(ZVirtualMachine* vm)
{
	// http://www.musicdsp.org/archive.php?classid=4#169
	// http://music.columbia.edu/pipermail/music-dsp/2007-June/066275.html
	// http://music.columbia.edu/pipermail/music-dsp/2004-May/060343.html
	// http://www.kvraudio.com/forum/viewtopic.php?f=33&t=362249&p=5099943&hilit=envelope+follower#p5099943
	// http://www.kvraudio.com/forum/viewtopic.php?f=33&t=368416&p=5179767&hilit=envelope+follower#p5179767

	auto decay     = vm->stack->Pop<num_t>();
	auto attack    = vm->stack->Pop<num_t>();
	auto ratio     = vm->stack->Pop<num_t>();
	auto threshold = dbToGain(vm->stack->Pop<num_t>());

	auto decayPerSample  = (decay  <= 0.0) ? 0.0 : zexpd(-1.0 / (kSampleRate * decay  / 1000.0)); // this yields Exp[-1] = 0.36 after decay time
	auto attackPerSample = (attack <= 0.0) ? 0.0 : zexpd(-1.0 / (kSampleRate * attack / 1000.0));

	auto envelopeTau = zexpd(-1.0 / (kSampleRate / 500.0));

	ZBlockBufferInternal& block = vm->stack->Pop<ZBlockBufferInternal>();

	double slopePower = 1 - 1.0/ratio;

	// NOTES:
	// The RMS tau should probably follow attack/decay somewhat to limit distortion

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		double s = 0.5*(block.samples[i].d[0]*block.samples[i].d[0] + block.samples[i].d[1]*block.samples[i].d[1]);

		rmsSquared = envelopeTau*rmsSquared + (1.0-envelopeTau)*s;
		double rms = zsqrtd(rmsSquared);

		double theta = (rms < envelope) ? decayPerSample : attackPerSample;
		envelope = theta*envelope + (1.0-theta)*rms;

		double gain = 1.0;
		if (envelope > threshold)
			gain = zpowd(envelope/threshold, -slopePower);

		block.samples[i] *= gain;
	}

	vm->stack->Push(block);

}

} // namespace invader
