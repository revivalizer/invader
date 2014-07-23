#pragma once

namespace invader {

class ZSynth;
class ZInstrument;
class ZVMProgram;
class ZOnepoleFilter;
class ZLevelFollower;

class ZVoice : public align16
{
public:
	ZVoice(ZSynth* synth, ZInstrument* instrument, ZVMProgram* program, section_id_t section, ZVMStorage* globalStorage);
	~ZVoice(void);

	void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples);
	void NoteOff(uint32_t deltaSamples);

	bool IsActive() const { return isActive; }
	bool IsNoteOn() const { return isNoteOn; }

	ZBlockBufferInternal& ProcessBlock(void);

	double pitch;
	uint32_t note, velocity, deltaSamples;

	double timeSinceNoteOn;
	double timeSinceNoteOff;
	bool   isNoteOn;

	ZOnepoleFilter* dcTrap;
	ZLevelFollower* levelFollower;

	ZVirtualMachine vm;

	opcode_index_t bytecodeStart, bytecodeEnd;

	ZVMProgram* program;

	section_id_t section;

private:
	bool isActive;
};

} // namespace invader
