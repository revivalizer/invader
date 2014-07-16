#include "pch.h"

namespace invader {

const double ZResampler2x::coeffd[] = {0.045362164348961023, 0.16808748123450229, 0.33714968797907396, 0.52237785430835393, 0.70806413636353871, 0.89744559117277234};

ZResampler2x::ZResampler2x(void)
{
	Reset();
	
	for (uint32_t i=0; i<numStages; i++)
	{
		//coeff[i].v = _mm_set1_pd(coeffd[i]);
		coeff[i] = sample_t(coeffd[i]);
	}
}

void ZResampler2x::Downsample(const sample_t& input0, const sample_t& input1, sample_t& output)
{
	sample_t out0, out1;

	Resample(input1, input0, out0, out1); // inputs swapped according to Laurent de Soras implementation

	output = (out0 + out1)*sample_t(0.5);
	//output.v = _mm_mul_pd(_mm_add_pd(out0.v, out1.v), _mm_set1_pd(0.5)); // result is average of values
}

// We could upsample by inputting the same sample twice, and taking the two different output signals

void ZResampler2x::Resample(const sample_t& input0, const sample_t& input1, sample_t& output0, sample_t& output1)
{
	sample_t* output[2] = { &output0, &output1 };
	*output[0] = input0;
	*output[1] = input1;

	for (uint32_t stage=0; stage<numStages; stage++)
	{
		uint32_t i = stage & 1; // Subfilter index (0 or 1)

		sample_t curIn = *output[i];

		// output = (input - y[stage])*coeffs[stage] + x[stage];
		//y[stage].v = output[i]->v = _mm_add_pd(_mm_mul_pd(_mm_sub_pd(curIn.v, y[stage].v), coeff[stage].v), x[stage].v);
		y[stage] = *output[i] = (curIn-y[stage])*coeff[stage] + x[stage];

		x[stage] = curIn;
	}
}

void ZResampler2x::Reset(void)
{
	for (uint32_t i=0; i<numStages; i++)
	{
		x[i] = y[i] = sample_t::zero();
		/*
		x[i].v = _mm_set1_pd(0);
		y[i].v = _mm_set1_pd(0);
		*/
	}
}

template<uint32_t oversamplingFactor>
void ZResampler2xDownsample(ZResampler2x& resampler, ZBlockBuffer<oversamplingFactor>& out, ZBlockBuffer<oversamplingFactor*2>& in)
{
	for (uint32_t i=0; i<out.numSamples; i++)
	{
		resampler.Downsample(in.samples[i*2], in.samples[i*2+1], out.samples[i]);
	}
}

template void ZResampler2xDownsample(ZResampler2x& resampler, ZBlockBuffer<kDefaultOversampling/2>& out, ZBlockBuffer<kDefaultOversampling>& in);

} // namespace invader