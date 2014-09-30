#pragma once

namespace invader {

enum
{
	kFilterTypeLP6 = 0,
	kFilterTypeLP12,
	kFilterTypeLP18,
	kFilterTypeLP24,
	kFilterTypeHP6,
	kFilterTypeHP12,
	kFilterTypeHP18,
	kFilterTypeHP24,
	kFilterTypeBP6_6,
	kFilterTypeBP6_12,
	kFilterTypeBP6_18,
	kFilterTypeBP12_6,
	kFilterTypeBP12_12,
	kFilterTypeBP18_6,
};

class ZFilterZD24
{
public:
	ZFilterZD24(void);
	~ZFilterZD24(void);

	void SetParameters(uint32_t type, double cutoff, double resonance);
	void Reset();
	void Process(ZBlockBufferInternal& block);

	uint32_t   type;
	double     cutoff;
	double     resonance;

	sample_t buffer[4];

	sample_t f, f2, t, g0, g1, g2, g3, g4, g5;
	sample_t c0, c1, c2, c3, c4;
};

} // namespace invader

