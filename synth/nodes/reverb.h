#pragma once

namespace invader {

class ZReverb : public ZNode
{
public:
	ZReverb(nodetype_t type);
	~ZReverb(void);

	virtual void Process(ZVirtualMachine* vm);

	virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples) { pitch; note; velocity; deltaSamples; }
	virtual void NoteOff(uint32_t deltaSamples) { deltaSamples; }

private:
	// Parameters
	/*uint32_t delayLengthSeries, feedbackMatrixType;
	double roomSize, halflife, attenuation;
	double rotationIn, rotationOut;
	double modulationRate, modulationDepth;
	double wet, dry;*/

	// Functions
	void UpdateDelayLengths(const uint32_t delayLengthSeries, const double roomSize);
	void UpdateFeedbackMatrix(const uint32_t feedbackMatrixType);
	void UpdateHalflife(double halflife, double attenuation);
	void UpdateRotations(double rotationIn, double rotationOut);
	//void UpdateModulation();

//	void Reset();
/*
	// Variables
	bool mustResetBuffer;
	bool mustUpdateDelayLengths;
	bool mustUpdateFeedbackMatrix;
	bool mustUpdateHalflife;
	bool mustUpdateRotations;
	*/

	double *delayLine[4];
	int32_t readHead[4];
	int32_t writeHead;
	uint32_t longestDelayLine;

	uint32_t delayLineLength;
	uint32_t delayLineLengthMask;

	double sampleDelay[4];

	double feedbackMatrix[4][4];

	double inputMatrix[2][4];
	double outputMatrix[4][2];

	// For gain and attenuation
	double filter[4];
	double gain[4];
	double beta[4];
	double alpha[4];

	// Modulation
	double modulationPhase;
	double modulationPhaseOmega;

	double modulationOffset[4];
	double modulationOffsetFrac[4];
	double modulationDelta[4];
};


} // namespace invader