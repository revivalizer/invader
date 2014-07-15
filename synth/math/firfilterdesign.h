#pragma once

class ZFIRFilterDesign
{
public:
	ZFIRFilterDesign(uint32_t N);
	~ZFIRFilterDesign(void);

	void SetBand(double min, double max, double idealValue, double maxError);

private:
	static const uint32_t oversampling = 16;
	uint32_t N;

	zvector<double> idealValue;
	zvector<double> maxError;
};

