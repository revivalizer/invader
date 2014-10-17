#pragma once

struct ZExeSong;

class ZExeSongRenderer
{
	public:
		ZExeSongRenderer(ZExeSong* song, invader::ZVMProgram* prog, short* buffer);

		// Vitals
		short* buffer;
		invader::ZSynth synth;
		ZExeSong* song;

		// Derived song data 
		double samplesPerLine;
		uint32_t numSamplesInSong;

		void Process(uint32_t endSample);

private:
		// Rendering state
		double sampleCount;
		double lineSampleCount;
		uint32_t currentLine;

		// Resampler
		invader::ZResampler2x downsampler;
		ZBlockBuffer1x downsampleBuffer;
};