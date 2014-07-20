#include "pch.h"

namespace invader {

	ZVoice::ZVoice( ZSynth* synth, ZInstrument* instrument, ZVMProgram* program, uint32_t section, ZVMStorage* globalStorage ) : isNoteOn(false)
		, isActive(false)
		, dcTrap(new ZOnepoleFilter)
		, levelFollower(new ZLevelFollower)
		, vm(program, new ZVMStack((uintptr_t)new uint8_t[10000]), globalStorage)
		, program(program)
		, section(section)
	{
	// Cutoff 5Hz, makes the trap fast enough to track offset changes,
	// but causes only a moderate attenuation of 0.26 dB and a phase shift of 14deg at 20 Hz
	dcTrap->SetCutoff(5); 

	vm.synth      = synth;
	vm.instrument = instrument;
	vm.voice      = this;

	bytecodeStart = program->sections[section];
	bytecodeEnd   = program->sections[section+1];

	vm.CreateNodeInstances(program, section); 
}

ZVoice::~ZVoice(void)
{
	delete dcTrap;
	delete levelFollower;
}

void ZVoice::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	isNoteOn = true;
	isActive = true;

	this->pitch        = pitch;
	this->note         = note;
	this->velocity     = velocity;
	this->deltaSamples = deltaSamples;

	timeSinceNoteOn  = 0.0;
	timeSinceNoteOff = 0.0;

	// Note on nodes
	for (opcode_index_t i=bytecodeStart; i<bytecodeEnd; i++)
	{
		if (vm.nodeInstances[i])
			vm.nodeInstances[i]->NoteOn(pitch, note, velocity, deltaSamples);
	}

	dcTrap->Reset();
	levelFollower->Reset();
}

void ZVoice::NoteOff(uint32_t deltaSamples)
{
	isNoteOn = false;

	// Note off nodes
	for (opcode_index_t i=bytecodeStart; i<bytecodeEnd; i++)
	{
		if (vm.nodeInstances[i])
			vm.nodeInstances[i]->NoteOff(deltaSamples);
	}
}

ZBlockBufferInternal& ZVoice::ProcessBlock(void)
{
	vm.Run(bytecodeStart, program);
	ZBlockBufferInternal& output = ZOnepoleFilterHighpassBlock(*dcTrap, vm.globalStorage->Load<ZBlockBufferInternal>((invader::opcode_index_t)(section*sizeof(ZBlockBufferInternal))));

	// Check level
	ZLevelFollowerProcessBlock(*levelFollower, output);

	if (!IsNoteOn() && levelFollower->GetdBLevel() < -108)
		isActive = false;

	if (IsNoteOn())
	{	
		timeSinceNoteOn  += kBlockSize*kDefaultOversampling/kSampleRate;
	}
	else
	{
		timeSinceNoteOn  += kBlockSize*kDefaultOversampling/kSampleRate;
		timeSinceNoteOff += kBlockSize*kDefaultOversampling/kSampleRate;
	}

	return output;
}

} // namespace invader
