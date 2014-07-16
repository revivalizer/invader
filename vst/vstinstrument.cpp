#include "pch.h"

#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced parameter

#include "VSTInstrument.h"
//#include "VSTEditor.h"
//#include "gui/vm/luavm.h"

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return new VSTInstrument(audioMaster);
}

VSTInstrument::VSTInstrument(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, 1, 0), align16()
//, vm(new LuaVM())
, downsampler(new invader::ZResampler2x)
//, vstEditor(nullptr)
{
	if (audioMaster)
	{
		setNumInputs(0);				// no inputs
		setNumOutputs(2);		// usually 2 outputs, left and right channel respectively
		canProcessReplacing();
		programsAreChunks(true);
		isSynth();
		setUniqueID('inva');
	}

	//strcpy(programName, "default");

/*
	// Init LuaVM
	#ifdef RUN_FROM_DISK
		ZScopedRegistryKey basePath("BasePath");

		// Set skin path
		char skinPath[512];
		sprintf(skinPath, "%stransgressor/vst/gui/skins/", basePath.str);
		vm->SetGlobalString("skinPath", skinPath);

		// Paths should be matched by shortcuts below
		vm->AddPackagePath("%s?.lua", basePath.str);
		vm->AddPackagePath("%sexternal/libs/lpeg/src/?.lua", basePath.str);
		vm->AddPackagePath("%stransgressor/vst/compiler/?.lua", basePath.str);
	#else
		// Set skin path
		ZScopedRegistryKey skinPath("SkinPath");
		vm->SetGlobalString("skinPath", skinPath.str);

		// Load embedded scripts and set preload shortcuts
		ZResource scripts("packedscripts.lua", "LUA");
		vm->DoString((const char* const)scripts.GetString());
		vm->AddPackageShortcut("external.libs.lpeg.src");
		vm->AddPackageShortcut("transgressor.vst.compiler");
	#endif

	vm->Require("libs.base_synth_gui.script.init");
	vm->Require("transgressor.vst.gui.script.root");

	// Create Editor
	vstEditor = new VSTEditor(this, vm);
	setEditor(vstEditor);
*/
	suspend();

//	vm->SetGlobalVariable("editor", "VSTEditor *", vstEditor);

	// Create Synth
	synth = new invader::ZSynth(nullptr);
//	vm->SetGlobalVariable("synth", "ZSynth *", synth);
//	vm->LoadDefaultProgram();

	/*// Create test program
	synth->synth = synth;
	synth->instrument = NULL;
	synth->voice = NULL;

	synth->program = new ZVMProgram;
	synth->program->bytecodeSize = 3;
	synth->program->bytecode = new opcode_t[3];
	synth->program->bytecode[0] = kOpcodeSynthRenderInstrument;
	synth->program->bytecode[1] = 0;
	synth->program->bytecode[2] = kOpcodeStop;
	synth->entryPoint = 0;

	synth->CreateNodeInstances(synth->program);

	synth->instruments[0]->hasProgram = true;
	synth->instruments[0]->isActive = true;


	for (uint32_t i=0; i<kNumVoices; i++)
	{
		ZVoice* voice = synth->instruments[0]->voices[i];

		voice->synth = synth;
		voice->instrument = synth->instruments[0];
		voice->voice = voice;

		voice->program = new ZVMProgram;
		voice->program->bytecodeSize = 2;
		voice->program->bytecode = new opcode_t[2];
		voice->program->bytecode[0] = kOpcodeSynthSinOsc;
		voice->program->bytecode[1] = kOpcodeStop;
		voice->entryPoint = 0;

		voice->CreateNodeInstances(voice->program);
	}*/

}

VSTInstrument::~VSTInstrument()
{
	delete synth;
//	delete vm;
	delete downsampler;
}

VstInt32 VSTInstrument::getChunk(void **data, bool isPreset)
{
//	uint32_t length;
//	vm->SaveChunk((char **)data, &length);
//	return (VstInt32)length;
	return 0;
}

VstInt32 VSTInstrument::setChunk(void *data, VstInt32 byteSize, bool isPreset)
{
	//vm->LoadChunk((char *)data, (uint32_t)byteSize);
	//vstEditor->RequestRedraw();
	return 0;
}

void VSTInstrument::resume()
{
/*	// Reset time info
	synth->timeInfo.pos = 0.0f;
	synth->timeInfo.bpm = 120.0f;
	synth->timeInfo.bps = synth->timeInfo.bpm / 60.0f;
	*/

	downsampler->Reset();
}

void VSTInstrument::suspend()
{
}

void VSTInstrument::processReplacing(float** inputs, float** outputs, VstInt32 sampleFrames)
{
	// Before rendering, set rounding flags, and denormal flags
	ZFPUState fpuState(ZFPUState::kRoundToZero);

	synth->renderMutex.Lock();

		eventQueueMutex.Lock();
			int32_t numSamplesToRender = ((int32_t)sampleFrames)-((int32_t)sampleBuffer.SamplesAvailable());

			// Get time info
			VstTimeInfo *timeInfo = getTimeInfo(kVstPpqPosValid | kVstTempoValid | kVstBarsValid | kVstCyclePosValid | kVstTimeSigValid | kVstTransportPlaying);

			synth->sync.bpm = (timeInfo && (timeInfo->flags & kVstTempoValid)) ? timeInfo->tempo : 120.0f;
			synth->sync.bps = synth->sync.bpm / 60.0f;

			bool shouldSync = timeInfo && (timeInfo->flags & kVstPpqPosValid) && (timeInfo->flags & kVstTransportPlaying);

			if (shouldSync)
			{
				synth->sync.pos = timeInfo->ppqPos;
				
				// Compensate for samples already generated
				synth->sync.pos += (double)sampleBuffer.SamplesAvailable() / kSampleRate * synth->sync.bps;
			}

			synth->sync.time = synth->sync.pos / synth->sync.bps;

			// Loop over blocks
			while (numSamplesToRender > 0)
			{
				// Handle relevant events
				while (!eventQueue.empty() && eventQueue.front().deltaFrames < kBlockSize)
				{
					VstMidiEvent event = eventQueue.front();
					char* midiData = event.midiData;
					zmsg("Midi Event: %02X %02X %02X\n", (unsigned char)midiData[0], (unsigned char)midiData[1], (unsigned char)midiData[2]);

					uint32_t channel = uint32_t(midiData[0]) & 0x0f;
					uint32_t status = uint32_t(midiData[0]) & 0xf0;

					if (status == 0x90 || status == 0x80) // Note on or note off
					{
						uint32_t note     = uint32_t(midiData[1]) & 0x7f;
						uint32_t velocity = uint32_t(midiData[2]) & 0x7f;

						if (status == 0x80)
							velocity = 0;	// note off by velocity 0

						if (velocity==0)
						{
							synth->NoteOff((uint32_t)channel, (uint32_t)note, (uint32_t)event.deltaFrames);
						}
						else
						{
							synth->NoteOn((uint32_t)channel, (uint32_t)note, (uint32_t)velocity, (uint32_t)event.deltaFrames);
						}
					}
					else if (status == 0xB0) // MIDI control change
					{
						uint32_t number = uint32_t(midiData[1]) & 0x7f;
						uint32_t value  = uint32_t(midiData[2]) & 0x7f;

						synth->ControlChange(channel, number, value, (uint32_t)event.deltaFrames);
					}

					eventQueue.pop_front();
				}

				// Render blocks
				synth->ProcessBlock();

				// Resample
				ZResampler2xDownsample(*downsampler, downsampleBuffer, synth->vm.stack->PopSampleBlock());

				sampleBuffer.PutBlock(downsampleBuffer);

				numSamplesToRender -= kBlockSize;

				// Decrement deltaSamples in events in queue
				for (event_queue_t::iterator it = eventQueue.begin(); it != eventQueue.end(); ++it)
					it->deltaFrames -= kBlockSize;
			}


		eventQueueMutex.Unlock();

	synth->renderMutex.Unlock();

	// Deinterleave output into buffers
	for (int32_t i=0; i<sampleFrames; i++)
	{
		sample_t sample = sampleBuffer.GetSample();
		outputs[0][i] = (float)sample.d[0];
		outputs[1][i] = (float)sample.d[1];
	}

	/*// We must always clear the buffer, to ensure non-garbage output
	Buffer dest;
	dest.channels[0]=outputs[0];
	dest.channels[1]=outputs[1];
	dest.numChannels=2;
	dest.numSamples=sampleFrames;

	dest.Reset(); // Only do this if replacing buffer

	// Note: This is mostly useful if some operation (e.g. synth realod) takes a while
	//       We are not guarenteed that synth has not been reloaded when we get to the lock
	if (synth->shouldRender==false)
	{
		return;
	}

	synth->renderMutex.Lock();

	// Recheck
	if (synth->shouldRender==false)
	{
		synth->renderMutex.Unlock();
		return;
	}

	// Get time info
	VstTimeInfo *timeInfo = getTimeInfo(kVstPpqPosValid | kVstTempoValid | kVstBarsValid | kVstCyclePosValid | kVstTimeSigValid | kVstTransportPlaying);

	synth->timeInfo.bpm = (timeInfo && (timeInfo->flags & kVstTempoValid)) ? timeInfo->tempo : 120.0f;
	synth->timeInfo.bps = synth->timeInfo.bpm / 60.0f;

	bool shouldSync = timeInfo && (timeInfo->flags & kVstPpqPosValid) && (timeInfo->flags & kVstTransportPlaying);

	if (shouldSync)
		synth->timeInfo.pos = timeInfo->ppqPos;


	// Process, but support samplesFrames other than 256
	synth->ProcessNon256(&dest);

	synth->renderMutex.Unlock();
	*/
	return;
}

VstInt32 VSTInstrument::processEvents(VstEvents* events)
{
	eventQueueMutex.Lock();
		for (VstInt32 i=0; i<events->numEvents; i++)
		{
			VstMidiEvent* event = (VstMidiEvent*)events->events[i];
			eventQueue.push_back(*event);
		}
	eventQueueMutex.Unlock();

	/*
	for (VstInt32 i=0; i<events->numEvents; i++)
	{
		if ((events->events[i])->type != kVstMidiType)
			continue;

		VstMidiEvent* event = (VstMidiEvent*)events->events[i];
		char* midiData = event->midiData;

		writeLog("Midi Event: %02X %02X %02X\n", (unsigned char)midiData[0], (unsigned char)midiData[1], (unsigned char)midiData[2]);

		VstInt32 channel = midiData[0] & 0x0f;

		// No, channel should really be zero :)
		channel=0;

		VstInt32 status = midiData[0] & 0xf0;

		if (status == 0x90 || status == 0x80)	// we only look at notes
		{
			VstInt32 note = midiData[1] & 0x7f;
			VstInt32 velocity = midiData[2] & 0x7f;
			if (status == 0x80)
				velocity = 0;	// note off by velocity 0

			if (velocity==0)
			{
				synth->NoteOff(note, event->deltaFrames);
			}
			else
			{
				synth->NoteOn(note, velocity, event->deltaFrames);
			}
		}
		else if (status == 0xb0)
		{
			// Control Change
			if (midiData[1] == 0x7e || midiData[1] == 0x7b)	// all notes off
			{
				synth->AllNotesOff(event->deltaFrames);
			}
			else if (midiData[1] == 0x01)
			{
				// Modwheel
				synth->SetModWheel((float)midiData[2] / 127.0f, event->deltaFrames);
			}
			else if (midiData[1] < 0x78)
			{
				scriptInterface->ControlChange(midiData[1], midiData[2]);
				((Editor*)editor)->RequestRedrawIfApplicable();
			}
		}
		else if (status == 0xe0)
		{
			// Pitch bend
			int msb = midiData[2] & 0x7f;
			int lsb = midiData[1] & 0x7f;

			float value = (float)(lsb + (msb << 7)) / 8192.f - 1.0f;

			synth->SetPitchBend(value, event->deltaFrames);


		}
		event++;
	}
	return 1;
	*/
	return 1;
}

void VSTInstrument::setSampleRate(float sampleRate)
{
/*	if (sampleRate!=synth->sampleRate)
		synth->SetSampleRate(sampleRate);

	scriptInterface->SetSampleRate(sampleRate);
	((Editor*)editor)->RequestRedrawIfApplicable();
	*/
}

bool VSTInstrument::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	strcpy (properties->label, "Stereo Out");
	properties->flags = kVstPinIsActive;
	properties->flags |= kVstPinIsStereo;
	properties->arrangementType = kSpeakerArrStereo;

	return true;
}

bool VSTInstrument::getEffectName(char* name)
{
	vst_strncpy(name, "invader", kVstMaxEffectNameLen);
	return true;
}

bool VSTInstrument::getVendorString(char* text)
{
	vst_strncpy (text, "revival", kVstMaxVendorStrLen);
	return true;
}

bool VSTInstrument::getProductString(char* text)
{
	vst_strncpy (text, "invader", kVstMaxProductStrLen);
	return true;
}

VstInt32 VSTInstrument::getVendorVersion()
{
	return 1;
}

VstInt32 VSTInstrument::canDo(char* text)
{
//	writeLog("Check if we can %s\n", text);
	if (!strcmp(text, "receiveVstEvents"))
		return 1;
	if (!strcmp(text, "receiveVstMidiEvent"))
		return 1;
	if (!strcmp(text, "midiProgramNames"))
		return 1;
	if (!strcmp(text, "receiveVstTimeInfo"))
		return 1;

	return -1;	// explicitly can't do; 0 => don't know
}

VstInt32 VSTInstrument::getNumMidiInputChannels()
{
	return 1; // we are monophonic
}

VstInt32 VSTInstrument::getNumMidiOutputChannels()
{
	return 0; // no MIDI output back to Host app
}

#pragma warning(pop)