#pragma once

namespace invader {

class ZInstrument;
class ZVMProgram;
class ZSynthVirtualMachine;

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
	ZInstrument* GetInstrumentFromMIDI(uint32_t channel, uint32_t note) { channel; note; return instruments[0]; }

	ZMutex renderMutex;

	ZVirtualMachine vm;

	ZSyncInfo sync; 

	opcode_index_t bytecodeStart, bytecodeEnd;
	ZVMProgram* program;

	uint32_t section;
};

} // namespace invader
