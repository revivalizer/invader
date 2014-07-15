#pragma once

namespace invader {

static const sample_t k1 = 1.0;
	
class ZOnepoleFilter : public align16
{
public:
	sample_t buf;
	sample_t f;

	ZINLINE ZOnepoleFilter(void)
	{
		Reset();
	}

	ZINLINE void Reset()
	{
		//buf.v = _mm_set1_pd(0);
		buf = sample_t::zero();
	}

	ZINLINE sample_t ZOnepoleFilter::Lowpass(const sample_t& input)
	{
		sample_t out = (buf + f*input) / (k1 + f);
		buf = f*(input-out) + out;
		return out;
		/*
		// out = (buf + f * input) / (1 + f); 
		sample_t out;
		out.v =	_mm_div_pd(
					_mm_add_pd(
						buf.v,
						_mm_mul_pd(
							f.v, 
							input.v)),
					_mm_add_pd(
						_mm_set1_pd(1),
						f.v));

	    // buf = f * (input - out) + out 
		buf.v = _mm_add_pd(
					_mm_mul_pd(
						f.v,
						_mm_sub_pd(
							input.v,
							out.v)), 
					out.v);

		return out;
		*/
	}

	ZINLINE sample_t ZOnepoleFilter::Highpass(const sample_t& input)
	{
		return input-Lowpass(input);
		/*
		sample_t out;
		out.v = _mm_sub_pd(input.v, Lowpass(input).v);
		return out;*/
	}

	ZINLINE void ZOnepoleFilter::SetCutoff(double cutoff)
	{
		f = sample_t(ztand(kM_PI * cutoff / kSampleRate));
		//double f_ = tan(M_PI * cutoff / kSampleRate);
		//f.v = _mm_set1_pd(f_);
	}
};

template<uint32_t oversamplingFactor>
ZBlockBuffer<oversamplingFactor>& ZOnepoleFilterLowpassBlock(ZOnepoleFilter& filter, ZBlockBuffer<oversamplingFactor>& block);

template<uint32_t oversamplingFactor>
ZBlockBuffer<oversamplingFactor>& ZOnepoleFilterHighpassBlock(ZOnepoleFilter& filter, ZBlockBuffer<oversamplingFactor>& block);

} // namespace invader
