#include "pch.h"

namespace invader {

//ZFIRInterpolator firInterpolator;

ZSynth::ZSynth(ZVMProgram* program)
	: vm(program, new ZVMStack((uintptr_t)zalignedalloc(10*1024*1024, 16)), new ZVMStorage((uintptr_t)zalignedalloc(program->globalStorageSize, 16)))
	, program(program)
	, numInstruments(program->numSections-1)
{
	firInterpolator.Init();

	instruments = new ZInstrument*[numInstruments];

	for (uint32_t i=0; i<numInstruments; i++)
	{
		instruments[i] = new ZInstrument(this, program, program->GetInstrumentSectionID(i), vm.globalStorage);
	}

	for (uint32_t i=0; i<kNumMIDIChannels; i++)
		midiChannelToInstrumentMap[i] = nullptr;

	section = program->GetMasterSectionID(); 

	vm.CreateNodeInstances(program, section); // last section is master section

	vm.synth      = this;
	vm.instrument = nullptr;
	vm.voice      = nullptr;

	// Execute const global section
	vm.Run(program->sections[program->GetConstGlobalSectionID()], program);
}

ZSynth::~ZSynth(void)
{
	// This is only done by synth, since only once globalstorage exists
	if (vm.globalStorage)
	{
		zalignedfree((void*)vm.globalStorage->mem);
		vm.globalStorage->mem = 0;

		delete vm.globalStorage;
		vm.globalStorage = nullptr;
	}

	if (instruments)
	{
		for (uint32_t i=0; i<numInstruments; i++)
		{
			if (instruments[i])
			{
				delete instruments[i];
				instruments[i] = nullptr;
			}
		}

		delete instruments;
		instruments = nullptr;
	}
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
	// Run global section
	vm.Run(program->sections[program->GetGlobalSectionID()], program);

	// Run instruments
	for (uint32_t i=0; i<numInstruments; i++)
	{
		instruments[i]->sync = sync;
		instruments[i]->ProcessBlock();
	}

	// Run master section
	vm.Run(program->sections[program->GetMasterSectionID()], program);

	sync.AdvanceBlock();
}

} // namespace invader
