#pragma once

struct ZExeSong;

class ZExeSongRenderer
{
	public:
		ZExeSongRenderer(ZExeSong* song, short* buffer);

		// Vitals
		short* buffer;
		ZSynth synth;
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
		ZResampler2x downsampler;
		ZBlockBuffer1x downsampleBuffer;
};