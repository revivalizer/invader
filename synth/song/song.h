#pragma once

struct ZExeSong
{
	enum { kNumInstruments = 16 };
	enum { kEventCCMask = 1 << 15 };

	// Convenience size of data
	uint32_t        size;

	// Song settings
	double          bpm;
	double          masterTrackVolume;

	// Counts
	uint32_t        numInstruments;
	uint32_t        numLines;
	uint32_t        numUniqueNoteEvents;
	uint32_t        numUniqueControlChangeEvents;
	uint16_t*       numEventsPerInstrument;

	// Event data
	uint16_t*       eventTime;
	uint16_t*       eventId;

	// Unique note event data
	uint8_t*        uneNote;	
	uint8_t*        uneVelocity;
	uint16_t*       uneLength;	

	// Unique control change event data
	uint16_t*       ucceNumber;
	uint16_t*       ucceValue;

	// ZVMProgram data
	ZVMProgram*     program;

	uint16_t        bytecodeLimitsCount;
	opcode_index_t* bytecodeLimits;

	ZExeSong() {};
	//ZExeSong(ZRenoiseSong* song);
	static uint32_t ComputeSize(ZExeSong* song);
	static ZExeSong* MakeBlob(ZExeSong* song);
	static ZExeSong* FromBlob(char* data) {return (ZExeSong*)data; }
	ZExeSong* Pack();
	ZExeSong* Unpack();
};

