#include "pch.h"

namespace invader {

ZInstrument::ZInstrument( ZSynth* synth, ZVMProgram* program, section_id_t section, ZVMStorage* globalStorage ) : synth(synth)
	, hasProgram(false)
	, isActive(false)
	, section(section)
	, globalStorage(globalStorage)
{
	for (uint32_t i=0; i<kNumVoices; i++)
	{
		voices[i] = new ZVoice(synth, this, program, section, globalStorage);
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

	if (voice!=0xFFFFFFFF)
	{
		double pitch = (double)note;

		voices[voice]->NoteOn(pitch, note, velocity, deltaSamples);
	}
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
	// Reset buffer in global storage
	ZBlockBufferInternal mix;
	mix.Reset();

	for (uint32_t i=0; i<kNumVoices; i++)
	{
		if (voices[i]->IsActive())
		{
			mix.Add(voices[i]->ProcessBlock());
		}
	}

	// Store mix, so it can be referenced by synth
	globalStorage->Store<ZBlockBufferInternal>((opcode_index_t)(section*sizeof(ZBlockBufferInternal)), mix);

	sync.AdvanceBlock();
}

} // namespace invader
