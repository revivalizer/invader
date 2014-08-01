#include "pch.h"

namespace invader { 

ZADSREnvelope::ZADSREnvelope() :
	analogueShapeTransform(0.93f),
	attackShaper(0.5f),
	decayShaper(0.5f),
	releaseShaper(0.5f)
{
	isInitialized = false;
}

void ZADSREnvelope::NoteOn()
{
	isInitialized = true;

	currentTime=0;

	noteOffed=false;

	attackShapeTransformed = analogueShapeTransform(attackShape) * 0.998 + 0.001;
	decayShapeTransformed = analogueShapeTransform(decayShape) * 0.998 + 0.001;
	releaseShapeTransformed = analogueShapeTransform(releaseShape) * 0.998 + 0.001;

	attackStart=GetNoteOnDelayValue();
	holdStart=attackStart + attack;
	decayStart=holdStart + GetHoldValue();
	fadeStart=decayStart + decay;

	if (attack > 0.0001) // Div by zero protection
	{
		attackExpFactor=zlogd(1.0-attackShapeTransformed)/attack;
		attackTimeInv = 1.0 / attack;
	}

	if (decay > 0.0001) // Div by zero protection
	{
		decayExpFactor=zlogd(1.0-decayShapeTransformed)/decay;
		decayTimeInv = 1.0 / decay;
	}

	fadeExpFactor=GetFadeValue()/6.0;

	attackInvScale=1.0 / attackShapeTransformed;
	decayInvScale=1.0 / decayShapeTransformed;


	if (!IsFadeValueZero())
		fadeTimeOffset = (6.0 * zlogd(sustain)) / (GetFadeValue() * zlogd(2.0));

	attackShaper = ShapingTransform(attackShape * 0.998 + 0.001);
	decayShaper = ShapingTransform(decayShape * 0.998 + 0.001);
}

void ZADSREnvelope::NoteOff()
{
	noteOffed=true;

	noteOffTime=(currentTime < fadeStart) ? fadeStart : currentTime; // only start noteoff after attack+hold+decay

	releaseStart=noteOffTime+GetNoteOffDelayValue();
	doneStart=releaseStart+release;

	releaseInvScale=1.0 / releaseShapeTransformed;

	if (release > 0.0001)
	{
		releaseExpFactor=zlogd(1.0-releaseShapeTransformed)/release;
		releaseTimeInv = 1.0 / release;
		releaseTimeOffset = zlogd((FadeValueAt(releaseStart) + 1.0/releaseShapeTransformed - 1.0) * releaseShapeTransformed) / releaseExpFactor;
		releaseTimeOffsetSoft = zlogd(FadeValueAt(releaseStart)) / releaseExpFactor;
	}
		
	double releaseShapeTransformedDigital = releaseShape * 0.998 + 0.001;
	releaseShaper = ShapingTransform(releaseShapeTransformedDigital);

	double fadeValueEnd = FadeValueAt(releaseStart);
	//releaseTimeOffsetShape = (fadeValueEnd + releaseShapeTransformedDigital - fadeValueEnd*releaseShapeTransformedDigital - 1.0) / (fadeValueEnd + releaseShapeTransformedDigital - 2.0*fadeValueEnd*releaseShapeTransformedDigital - 1.0);
	releaseTimeOffsetShape = - (1.0 - fadeValueEnd - releaseShapeTransformedDigital + fadeValueEnd*releaseShapeTransformedDigital) * release / (fadeValueEnd + releaseShapeTransformedDigital - 2.0*fadeValueEnd*releaseShapeTransformedDigital - 1.0);
}

bool ZADSREnvelope::IsFadeValueZero()
{
	if (GetFadeValue() < -0.01)
		return false;
	else if (GetFadeValue() > 0.01)
		return false;
	else
		return true;
}

double ClampedShapeValue(ShapingTransform &shape, double x)
{
	if (x < 0.0)
		return 0.0;
	else if (x > 1.0)
		return 1.0;
	else
		return shape(x);
}

double ZADSREnvelope::AttackValueAt(double time)
{
	if (type==kEnvelopeTypeAnalogue)
		return (1.0 - zexpd(attackExpFactor * (time - attackStart))) * attackInvScale;
	else
		return ClampedShapeValue(attackShaper, attackTimeInv * (time - attackStart));
}

double ZADSREnvelope::DecayValueAt(double time)
{
	if (type==kEnvelopeTypeAnalogue)
		return (1.0 - (1.0 - sustain) * (1.0 - zexpd(decayExpFactor * (time - decayStart))) * decayInvScale);
	else
		return 1.0 - (1.0 - sustain) * ClampedShapeValue(decayShaper, decayTimeInv * (time - decayStart));
}

double ZADSREnvelope::FadeValueAt(double time)
{
	if (IsFadeValueZero())
		return sustain;
	else
		return zpowd(2.0, fadeExpFactor * (time - fadeStart + fadeTimeOffset));	
}

double ZADSREnvelope::ReleaseValueAt(double time)
{
	if (releaseMode==kEnvelopeReleaseModeHard)
	{
		if (type==kEnvelopeTypeAnalogue)
		{
			double value = 1.0 - (1.0 - zexpd(releaseExpFactor * (time - releaseStart + releaseTimeOffset))) * releaseInvScale;

			if (value < 0.0)
				return 0.0;
			else
				return value;
		}
		else
		{
			return 1.0 - ClampedShapeValue(releaseShaper, releaseTimeInv * (time - releaseStart + releaseTimeOffsetShape));
		}
	}
	else
	{
		return zexpd(releaseExpFactor * ( time - releaseStart + releaseTimeOffsetSoft ));
	}
}

double ZADSREnvelope::ValueAt(double time)
{
	State state = StateAt(time);

	switch (state)
	{
		case StateInactive:
			return 0.0;
		case StateAttack:
			return AttackValueAt(time);
		case StateHold:
			return 1.0;
		case StateDecay:
			return DecayValueAt(time);
		case StateFade:
			return FadeValueAt(time);
		case StateRelease:
			return ReleaseValueAt(time);
		case StateDone:
			return 0.0;
		default:
			ZASSERT(!"ADSREnvelope::ValueAt unknown state");
			return 0.0;
	};
}

ZADSREnvelope::State ZADSREnvelope::StateAt(double time)
{
	if (time < attackStart)
		return StateInactive;
	else if (time < holdStart)
		return StateAttack;
	else if (time < decayStart)
		return StateHold;
	else if (time < fadeStart)
		return StateDecay;
	else if (!noteOffed || (noteOffed && time < releaseStart))
		return StateFade;
	else if ((releaseMode==kEnvelopeReleaseModeHard && time<doneStart) || releaseMode==kEnvelopeReleaseModeSoft)
		return StateRelease;
	else
		return StateDone;
}

double ZADSREnvelope::ComputeSuggestedModulationFrequency(int numSamplesInFrame)
{
	int currentState = StateAt(currentTime);
	int stateAtFrameEnd = StateAt(currentTime + (double)numSamplesInFrame/kSampleRate);

	if (currentState != stateAtFrameEnd)
		return 5000.f;

	switch (currentState)
	{
		case StateInactive:
		case StateHold:
		case StateFade:
		case StateDone:
			return 50.f;

		case StateAttack:
			return 100.f/attack;
		case StateDecay:
			return 100.f/decay;
		case StateRelease:
			return 100.f/release;
		default:
			ZASSERT(false);
			return 0.f;
	};
}

double ZADSREnvelope::Advance()
{
	double value=ValueAt(currentTime);
	currentTime += 1.0 / kSampleRate;
	return value;
}

double ZADSREnvelope::Advance(int samples)
{
	double value=ValueAt(currentTime);
	currentTime += samples / kSampleRate;
	return value;
}

double ZADSREnvelope::Next()
{
	return ValueAt(currentTime + 1.0/kSampleRate);
}

double ZADSREnvelope::Next(int samples)
{
	return ValueAt(currentTime + samples/kSampleRate);
}

} // namespace invader
