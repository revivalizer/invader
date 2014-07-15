#include "pch.h"

namespace invader {

ZInstrument::ZInstrument(ZSynth* synth, ZVMProgram* program)
	: synth(synth)
	, hasProgram(false)
	, isActive(false)
	//, outputBuffer(nullptr)
{
	for (uint32_t i=0; i<kNumVoices; i++)
	{
		voices[i] = new ZVoice(synth, this, program);
	}

	for (uint32_t i=0; i<128; i++)
	{
		midiCC[i] = 0;
	}
}

ZInstrument::~ZInstrument(void)
{
	for (uint32_t i=0; i<kNumVoices; i++)
		delete voices[i];
}

void ZInstrument::NoteOn(uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	uint32_t voice = GetVoiceFromPool(note, velocity, deltaSamples);

	double pitch = (double)note;

	voices[voice]->NoteOn(pitch, note, velocity, deltaSamples);

}

void ZInstrument::NoteOff(uint32_t note, uint32_t deltaSamples)
{
	NoteOffVoicesPlayingNote(note, deltaSamples);
}

void ZInstrument::ControlChange(uint32_t number, uint32_t value, uint32_t deltaSamples)
{
	deltaSamples;

	midiCC[number] = uint8_t(value);
}

uint32_t ZInstrument::GetVoiceFromPool(uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
  deltaSamples; velocity;

	double bestScore = 0;
	uint32_t bestVoice = 0xFFFFFFFF;

	for (uint32_t i=0; i<kNumVoices; i++)
	{
		const ZVoice& voice = *voices[i];

		double score = 0;
		
		// If voice isn't active, we can return immediately 
		if (voice.IsActive()==false)
			return i;

		if (voice.IsNoteOn()==false)
			score += 400;

		if (voice.note==note)
			score += 400;

		// Add amplitude score later
		//score+=(int)(expf(-voice->GetCurrentOutputAmplitude())*500.f);
		score += -voice.levelFollower->GetdBLevel()*4;

		if (score > bestScore)
		{
			bestScore = score;
			bestVoice = i;
		}
	}

	return bestVoice;
}

void ZInstrument::NoteOffVoicesPlayingNote(uint32_t note, uint32_t deltaSamples)
{
	for (uint32_t i=0; i<kNumVoices; i++)
	{
		if (voices[i]->IsNoteOn() && voices[i]->note==note)
			voices[i]->NoteOff(deltaSamples);
	}
}

void ZInstrument::ProcessBlock(void)
{
	// Generate voice mix
	voiceMixBuffer.Reset();

	for (uint32_t i=0; i<kNumVoices; i++)
	{
		if (voices[i]->IsActive())
		{
			/*// Render voice

			voices[i]->vm->Run(voices[i]->vmProgram->entryPoint, voices[i]->vmProgram);
			
			// Run DC trap
			// TODO: It's a little weird that the instrument has to do this... probably.
			ZOnepoleFilterHighpassBlock(*voices[i]->dcTrap, voices[i]->vm->blockStack->Get(-1));

			// Add to buffer
			mixBuffer.Add(voices[i]->vm->blockStack->Pop());*/

			voiceMixBuffer.Add(voices[i]->ProcessBlock());
		}
	}

	// Run program
	//Run(bytecodeStart, program);
	//outputBuffer = &(blockStack->Pop());

	sync.AdvanceBlock();
}

} // namespace invader
