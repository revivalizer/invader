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
