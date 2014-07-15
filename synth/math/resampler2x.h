#pragma once

class ZResampler2x : public align16
{
public:
	ZResampler2x(void);

	static const uint32_t numStages = 6;
	static const double coeffd[numStages];
	sample_t coeff[numStages];

	void Resample(const sample_t& input0, const sample_t& input1, sample_t& output0, sample_t& output1);
	void Downsample(const sample_t& input0, const sample_t& input1, sample_t& output);
	void Upsample(const sample_t& input0, const sample_t& input1, sample_t& output);

	void Reset(void);

private:
	sample_t x[numStages];
	sample_t y[numStages];

	static const double doeffd[6];
};

template<uint32_t oversamplingFactor>
void ZResampler2xDownsample(ZResampler2x& resampler, ZBlockBuffer<oversamplingFactor>& out, ZBlockBuffer<oversamplingFactor*2>& in);