#include "pch.h"
#include "song.h"

#define UnpackPointer(parameter)\
	parameter = (decltype(parameter)) ((uintptr_t)parameter + (uintptr_t)this);

#define PackPointer(parameter)\
	parameter = (decltype(parameter)) ((uintptr_t)parameter - (uintptr_t)this);

ZExeSong* ZExeSong::Pack()
{
	PackPointer(numEventsPerInstrument);

	PackPointer(eventTime);
	PackPointer(eventId);

	PackPointer(uneNote);
	PackPointer(uneVelocity);
	PackPointer(uneLength);

	PackPointer(ucceNumber);
	PackPointer(ucceValue);

	return this;
}

ZExeSong* ZExeSong::Unpack()
{
	UnpackPointer(numEventsPerInstrument);

	UnpackPointer(eventTime);
	UnpackPointer(eventId);

	UnpackPointer(uneNote);
	UnpackPointer(uneVelocity);
	UnpackPointer(uneLength);

	UnpackPointer(ucceNumber);
	UnpackPointer(ucceValue);

	return this;
}

uint32_t ZExeSong::ComputeSize(ZExeSong* song)
{
	// Compute song size
	uint32_t totalNumEvents = 0;

	// .. get total number of events
	for (uint32_t i=0; i<song->numInstruments; i++)
		totalNumEvents += song->numEventsPerInstrument[i];

	uint32_t size  = sizeof(ZExeSong)

	               + sizeof(song->numEventsPerInstrument[0]) * song->numInstruments

	               + sizeof(song->eventTime[0]) * totalNumEvents
	               + sizeof(song->eventId[0])   * totalNumEvents

	               + sizeof(song->uneNote[0])     * song->numUniqueNoteEvents
	               + sizeof(song->uneVelocity[0]) * song->numUniqueNoteEvents
	               + sizeof(song->uneLength[0])   * song->numUniqueNoteEvents

	               + sizeof(song->ucceNumber[0]) * song->numUniqueControlChangeEvents
	               + sizeof(song->ucceValue[0])  * song->numUniqueControlChangeEvents

				   ;
	return size;
}

#define AddArrayToPackedSong(parameter, num) \
	packedSong->parameter = decltype(song->parameter)(data);\
		\
	size = sizeof(song->parameter[0])*num;\
	zmemcpy(data, song->parameter, size);\
	data += size;

#define AddBufferToPackedSong(parameter, size) \
	packedSong->parameter = decltype(song->parameter)(data); \
	zmemcpy(data, song->parameter, size); \
	data += size;

ZExeSong* ZExeSong::MakeBlob(ZExeSong* song)
{
	size_t size;

	// Get size
	auto totalSize = ZExeSong::ComputeSize(song);

	// Get total number of events
	uint32_t totalNumEvents = 0;

	for (uint32_t i=0; i<song->numInstruments; i++)
		totalNumEvents += song->numEventsPerInstrument[i];

	// Create data with song pointer
	auto data = new uint8_t[totalSize];
	ZExeSong* packedSong = (ZExeSong*)data;

	// Set non-pointer values
	packedSong->size = totalSize;

	packedSong->bpm               = song->bpm;
	packedSong->masterTrackVolume = song->masterTrackVolume;

	packedSong->numInstruments                = song->numInstruments;
	packedSong->numLines                      = song->numLines;
	packedSong->numUniqueNoteEvents           = song->numUniqueNoteEvents;
	packedSong->numUniqueControlChangeEvents  = song->numUniqueControlChangeEvents;

	data += sizeof(ZExeSong);

	// Add arrays
	AddArrayToPackedSong(numEventsPerInstrument, song->numInstruments);

	AddArrayToPackedSong(eventTime, totalNumEvents);
	AddArrayToPackedSong(eventId,   totalNumEvents);

	AddArrayToPackedSong(uneNote,     song->numUniqueNoteEvents);
	AddArrayToPackedSong(uneVelocity, song->numUniqueNoteEvents);
	AddArrayToPackedSong(uneLength,   song->numUniqueNoteEvents);

	AddArrayToPackedSong(ucceNumber, song->numUniqueControlChangeEvents);
	AddArrayToPackedSong(ucceValue,  song->numUniqueControlChangeEvents);

	return packedSong;
}