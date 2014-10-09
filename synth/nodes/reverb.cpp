#include "pch.h"
#include "reverb.h"

namespace invader {

static const double speedOfSound = 330.0;

static const double reverbFeedbackMatrix[][4][4] =
{
  // householder 0
  {
	{  1, -1, -1, -1 },
	{ -1,  1, -1, -1 },
	{ -1, -1,  1, -1 },
	{ -1, -1, -1,  1 },
  },

  // householder 1
  {
    { -1,  1, -1, -1, },
    { -1, -1,  1, -1, },
    { -1, -1, -1,  1, },
    {  1, -1, -1, -1, },
  },

  // householder 2
  {
    { -1, -1,  1, -1, },
    { -1, -1, -1,  1, },
    {  1, -1, -1, -1, },
    { -1,  1, -1, -1, },
  },

  // householder 3
  {
    { -1, -1, -1,  1, },
    {  1, -1, -1, -1, },
    { -1,  1, -1, -1, },
    { -1, -1,  1, -1, },
  },

  // householder 4
  {
    { -1,  1, -1, -1, },
    {  1, -1, -1, -1, },
    { -1, -1, -1,  1, },
    { -1, -1,  1, -1, },
  },

  // householder 5
  {
    {  1, -1, -1, -1, },
    { -1, -1,  1, -1, },
    { -1,  1, -1, -1, },
    { -1, -1, -1,  1, },
  },

  // householder 6
  {
    { -1, -1,  1, -1, },
    {  1, -1, -1, -1, },
    { -1, -1, -1,  1, },
    { -1,  1, -1, -1, },
  },

  // householder 7
  {
    {  1,  1, -1,  1, },
    { -1,  1,  1,  1, },
    {  1,  1,  1, -1, },
    {  1, -1,  1,  1, },
  },

  // stautner 0
  {
    {  0,  1,  1,  0, },
    { -1,  0,  0, -1, },
    {  1,  0,  0, -1, },
    {  0,  1, -1,  0, },
  },

  // stautner 1
  {
    {  0,  1,  1,  0, },
    { -1,  0,  0, -1, },
    {  0,  1, -1,  0, },
    {  1,  0,  0, -1, },
  },

  // stautner 2
  {
    {  1,  0,  1,  0, },
    {  0, -1,  0, -1, },
    {  1,  0, -1,  0, },
    {  0,  1,  0, -1, },
  },

  // stautner 3
  {
    {  1,  0,  1,  0, },
    {  0, -1,  0, -1, },
    {  0,  1,  0, -1, },
    {  1,  0, -1,  0, },
  },

  // hadamard 0
  {
    {  1,  1,  1,  1, },
    { -1,  1, -1,  1, },
    { -1, -1,  1,  1, },
    {  1, -1, -1,  1, },
  },

  // hadamard 1
  {
    {  1,  1,  1,  1, },
    {  1, -1,  1, -1, },
    {  1,  1, -1, -1, },
    {  1, -1, -1,  1, },
  },

  // hadamard 2
  {
    {  1, -1, -1,  1, },
    {  1,  1, -1, -1, },
    {  1, -1,  1, -1, },
    {  1,  1,  1,  1, },
  },

  // hadamard 3
  {
    {  1, -1, -1,  1, },
    { -1, -1,  1,  1, },
    { -1,  1, -1,  1, },
    {  1,  1,  1,  1, },
  },

  // positive identity
  {
    {  1,  0,  0,  0, },
    {  0,  1,  0,  0, },
    {  0,  0,  1,  0, },
    {  0,  0,  0,  1, },
  },
/*
  // tiny rotation
  {
    0.980416164f, -0.121205295f, -0.121205295f, -0.096964236f,
    0.096964236f,  0.983109615f, -0.121205295f, -0.096964236f,
    0.121205295f,  0.096964236f,  0.980416164f, -0.121205295f,
    0.121205295f,  0.096964236f,  0.096964236f,  0.983109615f,
  },

  // little rotation
  {
    0.8271971909f, -0.3741443293f, -0.3741443293f, -0.1891474501f,
    0.1891474501f,  0.8879540606f, -0.3741443293f, -0.1891474501f,
    0.3741443293f,  0.1891474501f,  0.8271971909f, -0.3741443293f,
    0.3741443293f,  0.1891474501f,  0.1891474501f,  0.8879540606f,
  },

  // medium rotation
  {
    0.4027342996f, -0.7482592774f, -0.5147750269f, -0.1136640546f,
    0.1136640546f,  0.6111487307f, -0.7482592774f, -0.2317012589f,
    0.5147750269f,  0.1136640546f,  0.4027342996f, -0.7482592774f,
    0.7482592774f,  0.2317012589f,  0.1136640546f,  0.6111487307f,
  } ,*/

  // negative identity
  {
    { -1,  0,  0,  0, },
    {  0, -1,  0,  0, },
    {  0,  0, -1,  0, },
    {  0,  0,  0, -1, },
  },
  /*
  // negative tiny rotation
  {
    -0.980416164f,  0.121205295f,  0.121205295f,  0.096964236f,
    -0.096964236f, -0.983109615f,  0.121205295f,  0.096964236f,
    -0.121205295f, -0.096964236f, -0.980416164f,  0.121205295f,
    -0.121205295f, -0.096964236f, -0.096964236f, -0.983109615f,
  },

  // negative little rotation
  {
    -0.8271971909f,  0.3741443293f,  0.3741443293f,  0.1891474501f,
    -0.1891474501f, -0.8879540606f,  0.3741443293f,  0.1891474501f,
    -0.3741443293f, -0.1891474501f, -0.8271971909f,  0.3741443293f,
    -0.3741443293f, -0.1891474501f, -0.1891474501f, -0.8879540606f,
  },

  // negative medium rotation
  {
    -0.4027342996f,  0.7482592774f,  0.5147750269f,  0.1136640546f,
    -0.1136640546f, -0.6111487307f,  0.7482592774f,  0.2317012589f,
    -0.5147750269f, -0.1136640546f, -0.4027342996f,  0.7482592774f,
    -0.7482592774f, -0.2317012589f, -0.1136640546f, -0.6111487307f,
  } ,*/
};

static const double reverbFeedbackMatrixUnitize[] =
{
  // 1/2 for Householder reflections
  .5, .5, .5, .5,
  .5, .5, .5, .5,

  // 1/sqrt(2) for Stautner matrices
  0.70710678f,
  0.70710678f,
  0.70710678f,
  0.70710678f,

  // 1/2 for Hadamard matrices
  0.5,
  0.5,
  0.5,
  0.5,

  // 1 for identities and rotations
  1, 1, 1, 1, 1, 1, 1, 1
};

static const double delayLineSeries[][4] =
{
	{1.0000, 1.5811, 2.2177, 2.7207},
	{1.0000, 1.4194, 2.6223, 2.2401}
};

ZReverb::ZReverb(nodetype_t type) : ZNode(type)
{
	delayLineLength = 131072; // samplerate * 3 * 0.5
	                          // max delay is 2.7 x maxRoomSize, max room size should be 0.5 seconds
	delayLineLengthMask = delayLineLength - 1;

	for (uint32_t i=0; i<4; i++)
		delayLine[i] = new num_t[delayLineLength];

	// Init modulation and filter
	for (uint32_t i=0; i<4; i++)
	{
		filter[i] = 0.f;
		modulationOffset[i] = 0;
		modulationOffsetFrac[i] = 0;

		modulationPhase[i] = 0;
	}

	writeHead = 0;
}

ZReverb::~ZReverb( void )
{
	for (uint32_t i=0; i<4; i++)
	{
		delete delayLine[i];
		delayLine[i] = nullptr;
	}
}


void ZReverb::Process(ZVirtualMachine* vm)
{
	// Get parameters
	auto gainDry = dbToGain(vm->stack->Pop<num_t>());
	auto gainWet = dbToGain(vm->stack->Pop<num_t>());

	auto modulationDepth      = vm->stack->Pop<num_t>();
	auto modulationFreq       = vm->stack->Pop<num_t>();

	auto rotationOut      = vm->stack->Pop<num_t>();
	auto rotationIn       = vm->stack->Pop<num_t>();

	auto attenuation    = vm->stack->Pop<num_t>();
	auto halflife       = vm->stack->Pop<num_t>();
	auto roomSize       = zclamp(vm->stack->Pop<num_t>(), 0.0, 160.0);

	auto delayLengthSeries       = zitruncd(zclamp(vm->stack->Pop<num_t>(), 0.0, 1.0));
	auto feedbackMatrixType      = zitruncd(zclamp(vm->stack->Pop<num_t>(), 0.0, 16.0));

	// Update state
	UpdateDelayLengths(delayLengthSeries, roomSize);
	UpdateFeedbackMatrix(feedbackMatrixType);
	UpdateHalflife(halflife, attenuation);
	UpdateRotations(rotationIn, rotationOut);
	//UpdateModulation();

	// Compute write head positions
	for (uint32_t i=0; i<4; i++)
		readHead[i] = (writeHead - zitruncd(sampleDelay[i]));

	double read[4], write[4];

	double modulationPhaseDelta[4];
	for (uint32_t i=0; i<4; i++)
	{
		modulationPhaseDelta[i] = kM_PI2 * modulationFreq / kSampleRate * delayLineSeries[delayLengthSeries][i];
	}

	ZBlockBufferInternal& block = vm->stack->Pop<ZBlockBufferInternal>();

	// Read from delay line
	for (uint32_t n=0; n<block.numSamples; n++)
	{
		for (uint32_t i=0; i<4; i++)
		{
			modulationOffset[i]     = zsind(modulationPhase[i] + (double)i * kM_PI_2 ) * modulationDepth * sampleDelay[0];
			modulationOffsetFrac[i] = modulationOffset[i] - zifloord(modulationOffset[i]);

			int read0 = (readHead[i] + zifloord(modulationOffset[i]) + 0) & delayLineLengthMask;
			int read1 = (readHead[i] + zifloord(modulationOffset[i]) + 1) & delayLineLengthMask;

			read[i] = (1.0-modulationOffsetFrac[i])*delayLine[i][read0] + modulationOffsetFrac[i]*delayLine[i][read1];
		}

		// Compute output
		double outputLeft = read[0] * outputMatrix[0][0] + read[1] * outputMatrix[1][0] + read[2] * outputMatrix[2][0] + read[3] * outputMatrix[3][0];
		double outputRight = read[0] * outputMatrix[0][1] + read[1] * outputMatrix[1][1] + read[2] * outputMatrix[2][1] + read[3] * outputMatrix[3][1];

		// Apply feedback matrix
		for (int i=0; i<4; i++)
			write[i] = read[0] * feedbackMatrix[0][i] + read[1] * feedbackMatrix[1][i] + read[2] * feedbackMatrix[2][i] + read[3] * feedbackMatrix[3][i];

		// Add input signal
		double inputLeft  = block.samples[n].d[0];
		double inputRight = block.samples[n].d[1];

		for (int i=0; i<4; i++)
			write[i] += inputLeft * inputMatrix[0][i] + inputRight * inputMatrix[1][i];

		// Add filter and gain
		for (int i=0; i<4; i++)
			write[i] = gain[i] * (filter[i] += alpha[i] * (write[i] - filter[i] ));


		// Write to delay line
		for (int i=0; i<4; i++)
			delayLine[i][writeHead] = write[i];

		// Update read and write heads
		for (uint32_t i = 0; i<4; i++)
			readHead[i] = (readHead[i] + 1) & delayLineLengthMask;

		writeHead = (writeHead + 1) & delayLineLengthMask;

		// Write output
		block.samples[n].d[0] = gainWet * outputLeft  + gainDry * inputLeft;
		block.samples[n].d[1] = gainWet * outputRight + gainDry * inputRight;

		// Update modulation phase
		for (uint32_t i=0; i<4; i++)
		{
			modulationPhase[i] += modulationPhaseDelta[i];

			if (modulationPhase[i] > kM_PI2)
				modulationPhase[i] -= kM_PI2;
		}

	}

	vm->stack->Push(block);

}

void ZReverb::UpdateDelayLengths(const uint32_t delayLengthSeries, const double roomSize)
{
	for (int i=0; i<4; i++)
		sampleDelay[i] = delayLineSeries[delayLengthSeries][i] * kSampleRate * (roomSize / speedOfSound); // convert meter to seconds
}

void ZReverb::UpdateFeedbackMatrix(const uint32_t feedbackMatrixType)
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			feedbackMatrix[i][j] = reverbFeedbackMatrix[feedbackMatrixType][i][j] * reverbFeedbackMatrixUnitize[feedbackMatrixType];
		}
	}
}

void ZReverb::UpdateHalflife(double halflife, double attenuation)
{
	double dbHalving = -6.0;
	double dbNotZero = 0.01;	// So we don't do division by zero

	double halflife_LF = dbHalving / (halflife + dbNotZero);
	double halflife_HF = dbHalving / (attenuation + dbNotZero);

	for (int i=0; i<4; i++)
	{
		gain[i] = zpowd( 2.0, -double( sampleDelay[i] / kSampleRate ) / halflife_LF );
		beta[i] = zpowd( 2.0, -double( sampleDelay[i] / kSampleRate ) / halflife_HF ) / gain[i];
		alpha[i] = 2.0 * beta[i] / ( 1.0 + beta[i] );
	}
}

void ZReverb::UpdateRotations(double rotationIn, double rotationOut)
{
	const double deg2rad = 0.01745329;

	for (int i=0; i<4; i++)
	{
		inputMatrix[0][i] = zsind( deg2rad * ( rotationIn + i*90.0 ) );
		inputMatrix[1][i] = zcosd( deg2rad * ( rotationIn + i*90.0 ) );

		outputMatrix[i][0] = .5f * zsind( deg2rad * ( rotationOut + i*90.0 ) );
		outputMatrix[i][1] = .5f * zcosd( deg2rad * ( rotationOut + i*90.0 ) );
	}
}

/*void ZReverb::UpdateModulation()
{
	for (int i=0; i<4; i++)
	{
		// Compute delta
		double currentOffset = (double)modulationOffset[i] + modulationOffsetFrac[i];
		double nextOffset = zsind(modulationPhase + (double)i * kM_PI/2 ) * modulationDepth * sampleDelay[0];

		modulationDelta[i] = (nextOffset - currentOffset) / 32.0;

		// Update alpha given fraction
		double newBeta = beta[i] * ( 1.0 + 0.7f * ( modulationDelta[i] * modulationDelta[i] ) / ( 1e-9f + ( modulationDelta[i] * modulationDelta[i] ) ) );
		alpha[i] = 2.0 * newBeta / ( 1.0 + newBeta );
	}
}
*/
} // namespace invader
