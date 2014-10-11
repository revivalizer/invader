//double 

ZINLINE double pitchToFrequency(double pitch)
{
	return 440*zpowd(2, (pitch-69)/12);
}

ZINLINE double frequencyToPitch(double freq)
{
	return zlog2d(freq/440.0)*12 + 69;
}

ZINLINE double dbToGain(double dB)
{
	return zpowd(10.0, dB/20.0);
}

ZINLINE double gainTodB(double gain)
{
	return 20.0 * zlog10d(gain);
}

template<class T>
ZINLINE T zabs(const T& a)
{
	if (a < T(0))
		return -a;
	else
		return a;
}

template<class T>
ZINLINE T zmax(const T& a, const T& b)
{
	if (a > b)
		return a;
	else
		return b;
}

template<class T>
ZINLINE T zmin(const T& a, const T& b)
{
	if (a < b)
		return a;
	else
		return b;
}

template<class T>
ZINLINE T zclamp(const T& value, const T& min, const T& max)
{
	return zmax(min, zmin(max, value));
}

/*ZINLINE double zmax(double a, double b)
{
	if (a > b)
		return a;
	else
		return b;
}

ZINLINE double zmin(double a, double b)
{
	if (a < b)
		return a;
	else
		return b;
}

ZINLINE double zclamp(double value, double min, double max)
{
	return zmax(min, zmin(max, value));
}*/

// From this SO answer: http://stackoverflow.com/a/6566033
ZINLINE double zmin( double a, double b )
{
    // Branchless SSE min.
    _mm_store_sd( &a, _mm_min_sd(_mm_set_sd(a),_mm_set_sd(b)) );
    return a;
}

ZINLINE double zmax ( double a, double b )
{
    // Branchless SSE max.
    _mm_store_sd( &a, _mm_max_sd(_mm_set_sd(a),_mm_set_sd(b)) );
    return a;
}

ZINLINE double zclamp ( double val, double minval, double maxval )
{
    // Branchless SSE clamp.
    // return minss( maxss(val,minval), maxval );

    _mm_store_sd( &val, _mm_min_sd( _mm_max_sd(_mm_set_sd(val),_mm_set_sd(minval)), _mm_set_sd(maxval) ) );
    return val;
}

