#pragma once

namespace invader {

class ZInstrument;
class ZVMProgram;
class ZSynthVirtualMachine;

enum { kNumMIDIChannels = 16 };

class ZSynth : public align16
{
public:
	ZSynth(ZVMProgram* program);
	~ZSynth(void);

	void NoteOn(uint32_t channel, uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	void NoteOff(uint32_t channel, uint32_t note, uint32_t deltaSamples);

	void ControlChange(uint32_t channel, uint32_t number, uint32_t value, uint32_t deltaSamples);

	void ProcessBlock(void);

	uint32_t numInstruments;

	ZInstrument** instruments;
	ZInstrument* GetInstrument(uint32_t i) { return instruments[i]; }
	ZInstrument* GetInstrumentFromMIDI(uint32_t channel, uint32_t note) { note; return midiChannelToInstrumentMap[channel]; }

	ZInstrument* midiChannelToInstrumentMap[kNumMIDIChannels];

	ZMutex renderMutex;

	ZVirtualMachine vm;

	ZSyncInfo sync; 

	ZVMProgram* program;

	section_id_t section;
};

} // namespace invader
