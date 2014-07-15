#include "pch.h"
#include "filterzd24.h"


ZFilterZD24::ZFilterZD24(void)
{
}


ZFilterZD24::~ZFilterZD24(void)
{
}

static double typeCoeffs[][5] = {
	{  0.0,   1.0,   0.0,   0.0,   0.0, },   // kFilterTypeLP6:
	{  0.0,   0.0,   1.0,   0.0,   0.0, },   // kFilterTypeLP12:     
	{  0.0,   0.0,   0.0,   1.0,   0.0, },   // kFilterTypeLP18:     
	{  0.0,   0.0,   0.0,   0.0,   1.0, },   // kFilterTypeLP24:     
	{  1.0,  -1.0,   0.0,   0.0,   0.0, },   // kFilterTypeHP6:      
	{  1.0,  -2.0,   1.0,   0.0,   0.0, },   // kFilterTypeHP12:     
	{  1.0,  -3.0,   3.0,  -1.0,   0.0, },   // kFilterTypeHP18:     
	{  1.0,  -4.0,   6.0,  -4.0,   1.0, },   // kFilterTypeHP24:     
	{  0.0,   0.0,   1.0,  -2.0,   1.0, },   // kFilterTypeBP12_12:  
	{  0.0,   0.0,   0.0,   1.0,  -1.0, },   // kFilterTypeBP6_18:   
	{  0.0,   1.0,  -3.0,   3.0,  -1.0, },   // kFilterTypeBP18_6:   
	{  0.0,   0.0,   1.0,  -1.0,   0.0, },   // kFilterTypeBP6_12:   
	{  0.0,   1.0,  -2.0,   1.0,   0.0, },   // kFilterTypeBP12_6:   
	{  0.0,   1.0,  -1.0,   0.0,   0.0, },   // kFilterTypeBP6_6:    
};

void ZFilterZD24::SetParameters(uint32_t type, double cutoff, double resonance)
{
	this->type      = type;
	this->cutoff    = cutoff;
	this->resonance = resonance;

	f  = ztand(kM_PI * cutoff / kSampleRate);
	f2 = 1.0 / (1.0 + f);
	t  = 1.0 + f;

	g0 = t*t*t;
	g1 = f * t*t;
	g2 = f*f * t;
	g3 = f*f*f; 
	g4 = f*f*f*f;
	g5 = t*t*t*t + f*f*f*f * resonance*4.0;

	c0 = typeCoeffs[type][0];
	c1 = typeCoeffs[type][1];
	c2 = typeCoeffs[type][2];
	c3 = typeCoeffs[type][3];
	c4 = typeCoeffs[type][4];

}

void ZFilterZD24::Reset()
{
	buffer[0] = 0.0;
	buffer[1] = 0.0;
	buffer[2] = 0.0;
	buffer[3] = 0.0;
}

void ZFilterZD24::Process(ZBlockBufferInternal& block)
{
	static const sample_t k4 = sample_t(4.0);

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		auto out = (buffer[3] * g0 + buffer[2] * g1 + buffer[1] * g2 + buffer[0] * g3 + block.samples[i] * g4) / g5;

		auto input = block.samples[i] - k4*resonance*out;

		auto in1 = input - (f*input + buffer[0])*f2;
		auto out1 = buffer[0] + f*in1;
		buffer[0] = f*in1 + out1;
		//buffer[0] = clamp(buffer[0], -1.414, 1.414);

		auto in2 = out1 - (f*out1 + buffer[1])*f2;
		auto out2 = buffer[1] + f*in2;
		buffer[1] = f*in2 + out2;

		auto in3 = out2 - (f*out2 + buffer[2])*f2;
		auto out3 = buffer[2] + f*in3;
		buffer[2] = f*in3 + out3;

		auto in4 = out3 - (f*out3 + buffer[3])*f2;
		auto out4 = buffer[3] + f*in4;
		buffer[3] = f*in4 + out4;
		//buffer[3] = tanh(buffer[3]*0.5)*2.0;

		block.samples[i] = c0*input + c1*out1 + c2*out2 + c3*out3 + c4*out4;
	}
}