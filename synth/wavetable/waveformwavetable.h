#pragma once

namespace invader {

template <uint32_t size = 2048, uint32_t padFactor=1> // size is number of samples in table, padFacotr applied independently
class ZWaveformWavetable : public ZWavetable<size, padFactor>
{
public:
	ZWaveformWavetable(const ZRealSpectrum& spectrum)
		: spectrum(spectrum)
	{
	}
	
private:
	virtual ZWave<size>* Generate(uint32_t i)
	{
		uint32_t oct = i/12;
		uint32_t shift = (2^oct) >> 1; // 0, 1, 2, 4, 8, ...

		ZRealSpectrum shiftedSpectrum;

		for (uint32_t i=0; i<shift; i++)
			shiftedSpectrum.data[i] = 0.0;

		for (uint32_t i=shift; i<shiftedSpectrum.size; i++)
			shiftedSpectrum.data[i] = spectrum.data[i-shift];

		double wave[2048];

		complex::GFFT<2048>::realifft(shiftedSpectrum.data, wave);
		
		auto wave = new ZWave<size>;

		for (uint32_t i=0; i<2048; i++)
		{
			double clampedVal = zclamp(wave[i]*32767.0, -32767.0, 32676.0);
			wave->paddedData[i*2+0] = int16_t(clampedVal);
		}
	}

	ZRealSpectrum spectrum;
};

/*class ZWaveformWavetable
{
public:
	ZWaveformWavetable(const ZRealSpectrum& spectrum);
	~ZWaveformWavetable();
	
	virtual ZWave* GetWave(double pitch);

private:
	ZRealSpectrum spectrum;
	ZWave* table[11]; // 11*12 = 132 > 128
};*/

} // namespace invader