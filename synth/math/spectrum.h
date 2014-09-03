#pragma once

namespace invader {

template <class T, uint32_t S>
class ZSpectrum : public align16
{
public:
	T data[S];
	static const uint32_t size = S;
};

typedef ZSpectrum<complex_t, 2048> ZRealSpectrum;

} // namespace invader
