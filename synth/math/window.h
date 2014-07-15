#pragma once

template<class T>
class ZWindowFunction : public align16
{
public:
	ZWindowFunction(uint32_t numBins);
	~ZWindowFunction(void);

	zvector<T> bins;

	uint32_t numBins;

	void GenerateBlackmanNuttall();
};

template<class T>
ZWindowFunction<T>::ZWindowFunction(uint32_t numBins)
	: numBins(numBins)
	, bins(numBins)
{
	bins.reset(numBins);
}

template<class T>
ZWindowFunction<T>::~ZWindowFunction(void)
{
}

template<class T>
void ZWindowFunction<T>::GenerateBlackmanNuttall()
{
	double N = numBins;

	/*
		// Nuttal, first derivative
		const double a0 =  0.355768;
		const double a1 = -0.487396;
		const double a2 =  0.144232;
		const double a3 = -0.012604;
		*/

	// Blackman-Nuttal, first derivative
	const double a0 =  0.3635819;
	const double a1 = -0.4891775;
	const double a2 =  0.1365995;
	const double a3 = -0.0106411;


	for (uint32_t i=0; i<numBins; i++)
	{
		double n = i;

		bins[i] = T(a0 + a1*zcosd(2.0*kM_PI*n/(N-1.0)) + a2*zcosd(4.0*kM_PI*n/(N-1.0)) + a3*zcosd(6.0*kM_PI*n/(N-1.0)));  
	}
}

