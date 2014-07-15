#include "pch.h"

ZSynthVirtualMachine::ZSynthVirtualMachine(ZSynth* synth, ZInstrument* instrument, ZVoice* voice, double* stack, ZStack<ZBlockBufferInternal>* blockStack) : ZVirtualMachine(stack)
	, blockStack(blockStack)
	, synth(synth)
	, instrument(instrument)
	, voice(voice)
	, program(NULL)
	, entryPoint(kInvalidOpcodeIndex)
{
}

ZSynthVirtualMachine::~ZSynthVirtualMachine(void)
{
}

#include "transgressor/synth/math/adsrenvelope.h"

bool ZSynthVirtualMachine::ExecuteOpcode(opcode_t opcode, argument_t argument, ZVMProgram* program)
{
	// Check if base vm handles the opcode
	if (!ZVirtualMachine::ExecuteOpcode(opcode, argument, program))
	{
		switch (opcode)
		{
			case kOpSynthNOP:
				blockStack->Push(ZBlockBufferInternal());
				blockStack->Top().Reset();
				break;
			case kOpSynthRenderInstrument:
				blockStack->Push(*(synth->instruments[argument]->outputBuffer));
				break;
			case kOpSynthVoiceMix:
				blockStack->Push(instrument->voiceMixBuffer);
				break;
			case kOpCallFunc:
				{
					switch (argument)
					{
						case 2:
							stack[0] = voice->velocity;
							stack++;
							break;
						case 3:
							{
								// Init
								ZADSREnvelope env;

								env.attackShape   = 0.8;
								env.decayShape    = 0.2;
								env.releaseShape  = 0.8;

								env.fade = 0.0;
								env.hold = 0.0;

								env.type = kEnvelopeTypeDigital;

								env.releaseMode  = kEnvelopeReleaseModeHard;
								env.fadeHoldMode = kEnvelopeFadeHoldModeOff;
								env.delayMode    = kEnvelopeDelayModeOff;

								env.noteOnDelay  = 0.0;
								env.noteOffDelay = 0.0;

								// Note on
								env.attack  = stack[-4] / 1000.0;
								env.decay   = stack[-3] / 1000.0;
								env.sustain = stack[-2];

								env.NoteOn();
								env.Advance((const int)(voice->timeSinceNoteOn*kSampleRate));

								stack[-4] = env.Advance(0);

								if (!voice->IsNoteOn())
								{
									env.release   = stack[-1] / 1000.0;
									env.NoteOff();
									env.Advance((const int)(voice->timeSinceNoteOff*kSampleRate));

									stack[-4] = env.Advance(0);
								}

								stack -= 3;

								break;
							}
						case 4:
							// If voice, we MUST read pos from instrument, because we may not follow synth. This goes for below as well.
							if (instrument)
								stack[0] = instrument->sync.pos;
							else
								stack[0] = synth->sync.pos;
							stack++;
							break;
						case 5:
							if (instrument)
								stack[0] = instrument->sync.time;
							else
								stack[0] = synth->sync.time;
							stack++;
							break;
						case 6:
							stack[0] = voice->timeSinceNoteOn * synth->sync.bps;
							stack++;
							break;
						case 7:
							stack[0] = voice->timeSinceNoteOn;
							stack++;
							break;
						case 9: // Get instrument MIDI CC
							stack[-1] = instrument->midiCC[zifloord(stack[-1])];
							break;
						case 10: // bpm
							if (instrument)
								stack[0] = instrument->sync.bpm;
							else
								stack[0] = synth->sync.bpm;
							stack++;
							break;
						case 11: // bps
							if (instrument)
								stack[0] = instrument->sync.bps;
							else
								stack[0] = synth->sync.bps;
							stack++;
							break;
						case 12: // seconds per beat
							if (instrument)
								stack[0] = 1.0 / instrument->sync.bps;
							else
								stack[0] = 1.0 / synth->sync.bps;
							stack++;
							break;
						default:
							return false;
					}

					break;
				}
			default:
				return false;
		}
	}

	return true;
}

#include "synth/nodes/sinosc.h"
#include "synth/nodes/mixer.h"
#include "synth/nodes/adsrgain.h"
#include "synth/nodes/sawosc.h"
#include "synth/nodes/noiseosc.h"
#include "synth/nodes/filter.h"
#include "synth/nodes/supersaw.h"
#include "synth/nodes/reverb.h"
#include "synth/nodes/delay.h"

ZNode* ZSynthVirtualMachine::CreateNodeFromOpcode(nodetype_t type)
{
	ZNode* baseNode = ZVirtualMachine::CreateNodeFromOpcode(type);

	if (baseNode)
		return baseNode;

	switch (type)
	{
		case kOpSynthSinOsc:
			return new ZSinOsc(type);
		case kOpSynthMixer:
			return new ZMixer(type);
		case kOpSynthADSRGain:
			return new ZADSRGain(type);
		case kOpSynthSawOsc:
			return new ZSawOsc(type);
		case kOpSynthNoiseOsc:
			return new ZNoiseOsc(type);
		case kOpSynthFilter:
			return new ZFilter(type);
		case kOpSynthSuperSaw:
			return new ZSuperSaw(type);
		case kOpSynthReverb:
			return new ZReverb(type);
		case kOpSynthDelay:
			return new ZDelay(type);
		default:
			break;
	}

	return NULL;
}

void ZSynthVirtualMachine::AllocateNodeInstances( ZVMProgram* program )
{
	if (nodeInstances)
		delete[] nodeInstances;

	nodeInstances = new ZNode*[program->bytecodeSize];
	zzeromem(nodeInstances, program->bytecodeSize*sizeof(ZNode*));
}

void ZSynthNode::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
  pitch; note; velocity; deltaSamples;
}

void ZSynthNode::NoteOff(uint32_t deltaSamples)
{
  deltaSamples;
}
