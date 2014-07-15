#pragma once

enum {
	kEnvelopeTypeAnalogue = 0,
	kEnvelopeTypeDigital,
	kNumEnvelopeTypes,
};

enum {
	kEnvelopeReleaseModeSoft = 0,
	kEnvelopeReleaseModeHard,
	kNumEnvelopeReleaseModes,
};

enum {
	kEnvelopeFadeHoldModeOn = 0,
	kEnvelopeFadeHoldModeOff,
	kNumEnvelopeFadeHoldModes,
};

enum {
	kEnvelopeDelayModeOn = 0,
	kEnvelopeDelayModeOff,
	kNumEnvelopeDelayModes,
};

class ZADSREnvelope
{
	public:
		enum State {
			StateInactive,
			StateAttack,
			StateHold,
			StateDecay,
			StateFade,
			StateRelease,
			StateDone,
		};

		ZADSREnvelope();

		void NoteOn();
		void NoteOff();

		void SetSampleRate(double sampleRate);

		double Advance(int samples);
		double Advance();

		double Next(int samples);
		double Next();

		State StateAt(double time);

		// parameters
		double attack, decay, sustain, release;
		double attackShape, decayShape, releaseShape;
		double attackShapeTransformed, decayShapeTransformed, releaseShapeTransformed;
		int type, releaseMode, fadeHoldMode, delayMode;
		double fade, hold;
		double noteOnDelay, noteOffDelay;

		double ComputeSuggestedModulationFrequency(int numSamplesInFrame);

		// Shortcuts for values that depend on mode settings
		double GetFadeValue() { return (fadeHoldMode==kEnvelopeFadeHoldModeOn) ? fade : 0.; }
		double GetHoldValue() { return (fadeHoldMode==kEnvelopeFadeHoldModeOn) ? hold : 0.; }
		double GetNoteOnDelayValue() { return (delayMode==kEnvelopeDelayModeOn) ? noteOnDelay : 0.; }
		double GetNoteOffDelayValue() { return (delayMode==kEnvelopeDelayModeOn) ? noteOffDelay : 0.; }

	private:
		double AttackValueAt(double time);
		double DecayValueAt(double time);
		double FadeValueAt(double time);
		double ReleaseValueAt(double time);

		double ValueAt(double time);

		bool IsFadeValueZero();

		double currentTime;
		bool noteOffed;

		double noteOffTime;

		double attackStart, holdStart, decayStart, fadeStart, releaseStart, doneStart;

		double attackExpFactor, decayExpFactor, fadeExpFactor, releaseExpFactor;
		double attackInvScale, decayInvScale, releaseInvScale;

		double attackTimeInv, decayTimeInv, releaseTimeInv;

		double fadeTimeOffset, releaseTimeOffset, releaseTimeOffsetShape, releaseTimeOffsetSoft;

		bool isInitialized;

		ShapingTransform analogueShapeTransform;

		ShapingTransform attackShaper, decayShaper, releaseShaper;

	friend class ASDRGain;
	friend class Envelope;
	friend class ADSRVisualFeedbackDrawer;
};