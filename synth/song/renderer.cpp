#include "pch.h"
#include "renderer.h"
#include "song.h"

/*void CreateNodeInstances(ZSynthVirtualMachine* vm, ZVMProgram* program, opcode_index_t entryPoint, opcode_index_t exitPoint)
{
	vm->AllocateNodeInstances(program);
	ZVMBytecodeIterator it(*program);

	while (it.Next())
	{
		bool isNode     = (it.opcode & kOpcodeMaskIsNode) != 0;
		bool isInLimits = it.i >= entryPoint && it.i<exitPoint;

		if (isNode && isInLimits)
		{
			vm->SetNodeInstance(it.i, vm->CreateNodeFromOpcode(it.opcode));
		}
	}
}

void InitVirtualMachine(ZSynthVirtualMachine* vm, ZVMProgram* program, opcode_index_t entryPoint, opcode_index_t exitPoint)
{
	CreateNodeInstances(vm, program, entryPoint, exitPoint);
	vm->program = program;
	vm->entryPoint = entryPoint;
}*/

ZExeSongRenderer::ZExeSongRenderer(ZExeSong* song, invader::ZVMProgram* prog, short* buffer)
	: sampleCount(0.0)
	, lineSampleCount(samplesPerLine)
	, currentLine(0)
	, buffer(buffer)
	, song(song)
	, synth(prog)
{
	// Set derived song information
	static const uint32_t linesPerBeat = 4;
	double bps = song->bpm/60.0;

	samplesPerLine = 44100.0 / (bps * linesPerBeat);
	numSamplesInSong = (uint32_t(double(song->numLines * samplesPerLine)) + 15) & ~0xF; // round up to nearest 16

	/*// Create node instances
	// We init in bytecode limit order so we can just walk thorugh the bytecode limit array
	auto limit = song->bytecodeLimits;

	InitVirtualMachine(&synth, song->program, limit[0], limit[1]);
	limit++;

	for (uint32_t instrument=0; instrument<song->numInstruments; instrument++)
	{
		InitVirtualMachine(synth.GetInstrument(instrument), song->program, limit[0], limit[1]);
		limit++;

		for (uint32_t voice=0; voice<kNumVoices; voice++)
		{
			ZVoice* v = synth.GetInstrument(instrument)->GetVoice(voice);

			InitVirtualMachine(v, song->program, limit[0], limit[1]);
		}

		limit++;
	}*/

	// Set synth sync info
	synth.sync.bpm  = song->bpm;
	synth.sync.bps  = bps;
	synth.sync.pos  = 0.0;
	synth.sync.time = 0.0;
}

void ZExeSongRenderer::Process( uint32_t endSample )
{
	while (sampleCount < endSample)
	{
		if (lineSampleCount >= samplesPerLine)
		{
			if (currentLine < song->numLines)
			{
				uint32_t event = 0;

				// Loop over all events in song and check for match (also does on the fly delta decoding of events)
				for (uint32_t instrument = 0; instrument < song->numInstruments; instrument++)
				{
					uint32_t lastLine = 0;

					for (uint32_t i=0; i<song->numEventsPerInstrument[instrument]; i++)
					{
						uint32_t uniqueEventId = song->eventId[event];

						bool isControlChangeEvent = (uniqueEventId & ZExeSong::kEventCCMask) != 0;

						if (!isControlChangeEvent)
						{
							uint32_t noteOnLine = lastLine + song->eventTime[event];
							uint32_t noteOffLine = noteOnLine + song->uneLength[uniqueEventId];

							if (noteOffLine==currentLine)
								synth.NoteOff(instrument, song->uneNote[uniqueEventId], 0);
							if (noteOnLine==currentLine)
								synth.NoteOn(instrument, song->uneNote[uniqueEventId], song->uneVelocity[uniqueEventId], 0);
						}
						else
						{
							uint32_t ccLine = lastLine + song->eventTime[event];

							uniqueEventId = uniqueEventId & (~ZExeSong::kEventCCMask);

							if (ccLine==currentLine)
								synth.ControlChange(instrument, song->ucceNumber[uniqueEventId], song->ucceValue[uniqueEventId], 0);
						}

						lastLine += song->eventTime[event];
						event++;
					}
				}
			}

			lineSampleCount -= samplesPerLine;
			currentLine++;
		}

		// Generate sound
		synth.ProcessBlock();

		// Resample
		ZResampler2xDownsample(downsampler, downsampleBuffer, synth.vm.stack->Pop<ZBlockBufferInternal>());

		// Set output
		sample_t gain = 32767.0*song->masterTrackVolume;

		for (uint32_t s=0; s<downsampleBuffer.numSamples; s++)
		{
			sample_t sample = downsampleBuffer.samples[s]*gain;

			*buffer++ = short(zifloord(zclamp(sample.d[0], -32676.0, 32676.0)));
			*buffer++ = short(zifloord(zclamp(sample.d[1], -32676.0, 32676.0)));
		}

		// Update sample position
		lineSampleCount += downsampleBuffer.numSamples;
		sampleCount += downsampleBuffer.numSamples;
	}
}
