#pragma once

namespace invader {

class ZSynth;
class ZVoice;

class ZInstrument : public align16
{
public:
	ZInstrument(ZSynth* synth, ZVMProgram* program);
	~ZInstrument(void);

	void NoteOn(uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	void NoteOff(uint32_t note, uint32_t deltaSamples);
	
	void ControlChange(uint32_t number, uint32_t value, uint32_t deltaSamples);

	void ProcessBlock(void);

	ZVoice* voices[kNumVoices];
	ZVoice* GetVoice(uint32_t i) { return voices[i]; }

	bool IsActive() const { return hasProgram && isActive; }

	ZBlockBufferInternal voiceMixBuffer;
	//ZBlockBufferInternal* outputBuffer;

	ZSyncInfo sync; 

	uint8_t midiCC[128];

private:
	uint32_t GetVoiceFromPool(uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	void NoteOffVoicesPlayingNote(uint32_t note, uint32_t deltaSamples);

	ZSynth* synth;

public:
	bool hasProgram;
	bool isActive;
};

} // namespace invader
