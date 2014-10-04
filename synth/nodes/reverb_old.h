#pragma once

#include "libs/base_synth/constants.h"

class ZDiffusionChain
{
public:
	ZDiffusionChain(double s1, double k1, double s2, double k2, double s3, double k3)
	{
		d1.size = zifloord(s1*kSampleRate/1000.0);
		d1.k    = sample_t(k1);

		d2.size = zifloord(s2*kSampleRate/1000.0);
		d2.k    = sample_t(k2);

		d3.size = zifloord(s3*kSampleRate/1000.0);
		d3.k    = sample_t(k3);
	}

	void Reset()
	{
		//d1.Reset();
		//d2.Reset();
		//d3.Reset();
	}

	sample_t Process(const sample_t& in)
	{
		return d1.Process(d2.Process(d3.Process(in)));
	}

	ZDiffuser<32768> d1;
	ZDiffuser<32768> d2;
	ZDiffuser<32768> d3;
};


typedef ZCircularBuffer<65536> ZLoopBuffer;

class ZGrain
{
public:
	ZWindowFunction<sample_t> window;

	double startOffset;
	double endOffset;
	double time;
	double length;
	double fadeLength;

	ZRandom r;

	bool mustReset;


	/*double fade(double a, double b, double x)
	{
		if (x < a)
			return 0.0;
		else if (x > b)
			return 1.0;
		else
		{
			x = (x - a) / (b - a);
			return x*x * (3.0 - 2.0*x);
			//return zsqrtd(x);
		}
	}*/

	ZGrain(uint32_t seed)
		: mustReset(true)
		, startOffset(0)
		, endOffset(0)
		, r(seed) // randomness based on this pointer
		, window((zifloord(kSampleRate*(0.30)) + 1) & ~1)
		, fadeLength(window.numBins/2)
		, length(window.numBins)
	{
		window.GenerateBlackmanNuttall();
	}

	void Reset(double resetPos, double bufferLength, double modRate)
	{
		//startOffset = (0.1 + 0.8*double(r.NextUniformFloat()))*bufferLength;
		startOffset = zclamp(resetPos - (0.0025 + 0.0050*r.NextUniformFloat())*kSampleRate, 1.0, bufferLength-1);

		endOffset = -1;

		//length = kSampleRate * 0.04*0.5 + kSampleRate * r.NextUniformFloat()*0.02*0.5 + 2.0*fadeLength;
		length = window.numBins;

		while (endOffset < 0.0 || endOffset > (bufferLength-1))
			endOffset = startOffset + double(r.NextGaussianFloat())*modRate*500.0*length/kSampleRate;

		time = 0.0;

		mustReset = false;
	}

	sample_t Update(double resetPos, double bufferLength, ZLoopBuffer* buffer, double modRate)
	{
		if (mustReset)
		{
			Reset(resetPos, bufferLength, modRate);
			time = zifloord(double(r.NextUniformFloat())*length);
		}

		auto windowVal = window.bins[zclamp(zifloord(time), 0, zifloord(length)-1)];
		auto pos = startOffset + (endOffset - startOffset)*(time/length);
		pos = zclamp(pos, 1.0, bufferLength-1.0);
		//auto pos = startOffset;

		time++;

		//if (time >= length)
		//	mustReset = true;

		// sample
		uint32_t ipos = zifloord(pos);
		auto fracpos = sample_t(pos - ipos);

		sample_t s0 = buffer->ReadOffsetSample(ipos+1);
		sample_t s1 = buffer->ReadOffsetSample(ipos);

		return sample_t(windowVal)*(s1*(sample_t(1.0)-fracpos) + s0*fracpos);
	}
};


class ZGrainPair
{
public:
	ZGrain* grain0, * grain1;

	ZGrainPair()
		: grain0(nullptr)
		, grain1(nullptr)
	{
	}

	void init(uint32_t seed)
	{
		ZRandom r(seed);
		grain0 = new ZGrain(r.NextUniformInt());
		grain1 = new ZGrain(r.NextUniformInt());
	}

	~ZGrainPair()
	{
		if (grain0)
		{
			delete grain0;
			grain0 = nullptr;
		}
		if (grain1)
		{
			delete grain1;
			grain1 = nullptr;
		}
	}

	sample_t Update(double resetPos, double bufferLength, ZLoopBuffer* buffer, double modRate)
	{
		sample_t out = grain0->Update(resetPos, bufferLength, buffer, modRate);

		if (grain0->time >= grain0->length-grain0->fadeLength)
			out += grain1->Update(resetPos, bufferLength, buffer, modRate);

		if (grain0->time >= grain0->length)
		{
			// Swap
			ZGrain* t = grain0;
			grain0 = grain1;
			grain1 = t;

			grain1->Reset(resetPos, bufferLength, modRate);
		}

		return out;
	}
};

class ZReverb : public ZSynthNode
{
public:
	ZReverb(nodetype_t type);
	virtual void Process(ZVirtualMachine* vm, argument_t argument);

	static const uint32_t kNumLoops = 3;
	static const uint32_t kTapsPerLoop = 3;

	ZLoopBuffer loop[kNumLoops];

	ZGrainPair grain[kNumLoops][kTapsPerLoop];

	ZDiffusionChain diffusorLeft, diffusorRight;

	ZOnepoleFilter inputFilter1;
	ZOnepoleFilter inputFilter2;

	ZCircularBuffer<32768> preDelayBuffer;

};

