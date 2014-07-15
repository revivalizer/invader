#include "pch.h"

namespace invader {

ZSynth::ZSynth(ZVMProgram* program)
	: vm(program, new ZVMStack((uintptr_t)new uint8_t[10000]))
	, program(program)
{
	for (uint32_t i=0; i<kNumInstruments; i++)
	{
		instruments[i] = new ZInstrument(this, program);
	}
}

ZSynth::~ZSynth(void)
{
	for (uint32_t i=0; i<kNumInstruments; i++)
		delete instruments[i];
}

void ZSynth::NoteOn(uint32_t channel, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	GetInstrumentFromMIDI(channel, note)->NoteOn(note, velocity, deltaSamples);
}

void ZSynth::NoteOff(uint32_t channel, uint32_t note, uint32_t deltaSamples)
{
	GetInstrumentFromMIDI(channel, note)->NoteOff(note, deltaSamples);
}

void ZSynth::ControlChange(uint32_t channel, uint32_t number, uint32_t value, uint32_t deltaSamples)
{
	channel; number; value; deltaSamples;
	//GetInstrumentFromMIDI(channel, note)->ControlChange(number, value, deltaSamples);
}

void ZSynth::ProcessBlock(void)
{
	for (uint32_t i=0; i<kNumInstruments; i++)
	{
		instruments[i]->sync = sync;
		instruments[i]->ProcessBlock();
	}

	vm.Run(bytecodeStart, program);

	sync.AdvanceBlock();
}

} // namespace invader
