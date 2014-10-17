#include "pch.h"

using namespace std;
using namespace invader;

ZRenoiseSong::ZRenoiseSong(void)
	: xmlData(nullptr)
	, xmlDataLength(0)
{
}

ZRenoiseSong::~ZRenoiseSong(void)
{
}

int ZRenoiseSong::Read(const char* path)
{
	int error = LoadRenoiseFile(path, &xmlData, &xmlDataLength);

	if (error==0)
	{
		MakeUniformLineEndings(xmlData, xmlDataLength);
		TiXmlDocument doc;
		doc.SetTabSize(0);
		doc.Parse(xmlData);

		if (doc.Error())
		{
			printf("Couldn't parse Song.xml\n");
			return -10;
		}
		else
		{
			ConvertXMLToSong(doc);
			return 0;
		}
	}
	else
	{
		return error;
	}
}

int ZRenoiseSong::LoadRenoiseFile(const char* path, char** data, int* dataLength)
{
	int error;

	unzFile zipFile = unzOpen(path);
	if (zipFile == nullptr)
	{
		printf("%s: not found\n", path);
		return -2;
	}
	else
	{
		error = unzLocateFile(zipFile, "Song.xml", true);

		if (error==UNZ_OK)
		{
			error = unzOpenCurrentFile(zipFile);

			unz_file_info info;
			unzGetCurrentFileInfo(zipFile, &info, nullptr, 0, nullptr, 0, nullptr, 0);

			if (error==UNZ_OK)
			{
				*data = new char[info.uncompressed_size+1];

				*dataLength = unzReadCurrentFile(zipFile, *data, info.uncompressed_size+1);

				if (*dataLength >= 0)
				{
					(*data)[*dataLength] = '\0';
					printf("%d bytes read\n", *dataLength);
					unzClose(zipFile);
					return 0;
				}
				else
				{
					unzClose(zipFile);
					printf("Couldn't read data from Song.xml.\n");
					return -5;
				}
			}
			else
			{
				unzClose(zipFile);
				printf("Couldn't open Song.xml in archive.\n");
				return -4;
			}
		}
		else
		{
			unzClose(zipFile);
			printf("Could not locate 'Song.xml' in archive\n", path);
			return -3;
		}
	}
}

void ZRenoiseSong::MakeUniformLineEndings(char* buf, int length)
{
	const char* p = buf;	// the read head
	char* q = buf;			// the write head
	const char CR = 0x0d;
	const char LF = 0x0a;

	buf[length] = 0;
	while( *p ) {
		assert( p < (buf+length) );
		assert( q <= (buf+length) );
		assert( q <= p );

		if ( *p == CR ) {
			*q++ = LF;
			p++;
			if ( *p == LF ) {		// check for CR+LF (and skip LF)
				p++;
			}
		}
		else {
			*q++ = *p++;
		}
	}
	assert( q <= (buf+length) );
	*q = 0;

}

struct NoteColumnState
{
	NoteColumnState() { SetInactive(); }

	void SetActive(int newInstrument, int newNote) { instrument = newInstrument; note = newNote; }
	void SetInactive() { instrument = -1; note = -1; }
	bool IsActive() { return instrument >= 0 && note >= 0; }

	ZRenoiseEvent* event;

	int instrument;
	int note;
};

struct TrackState
{
	NoteColumnState noteColumn[20];
};

//                    A  B   C  D  E  F  G
int letterToNote[] = {9, 11, 0, 2, 4, 5, 7};

int StrToNote(const char *str)
{
	if (str[0]=='O')
		return -1;	// Note off

	int octave = str[2]-'0';

	int note = octave*12 + letterToNote[str[0]-'A'];

	if (str[1]=='#')
		note++;
	
	return note;
}

unsigned int HexToInt(const char *str)
{
	unsigned int value;

	if ( sscanf(str, "%X", &value))
		return value;
	else
		return 0;
}


void ZRenoiseSong::ConvertXMLToSong(TiXmlDocument& doc)
{
	// Internal instrument vector, which includes NULL instruments
	vector<vector<int> > controlChangeValues;
	std::vector<int> instrumentChannel; // maps instrument id to midi channel

	TiXmlHandle root = TiXmlHandle(&doc).FirstChild("RenoiseSong");; 	

	// First, get instrument data

	// ... loop over instruments
	TiXmlElement *instrument = root.FirstChild("Instruments").FirstChild("Instrument").ToElement();

	int instrumentNum = 0;

	for (instrument; instrument; instrument=instrument->NextSiblingElement())
	{
		TiXmlElement *chunkData = TiXmlHandle(instrument).FirstChild("PluginProperties").FirstChild("PluginDevice").FirstChild("ParameterChunk").ToElement();
		TiXmlElement *name = TiXmlHandle(instrument).FirstChild("Name").ToElement();
		TiXmlElement *channel = TiXmlHandle(instrument).FirstChild("PluginProperties").FirstChild("Channel").ToElement();

		if (chunkData)
		{
#if 0
			// Get chunk string
			chunkStr = new char[strlen(chunkData->GetText()) + 1];
			int chunkStrLen = decode_base64((unsigned char*)chunkStr, chunkData->GetText());
			chunkStr[chunkStrLen] = '\0'; // TODO: I'm not entirely sure this is correct. Verify with Lua.

			LuaVM vm;
			ZScopedRegistryKey basePath("BasePath");

			// Paths should be matched by shortcuts below
			vm.AddPackagePath("%s?.lua", basePath.str);
			vm.AddPackagePath("%sexternal/libs/lpeg/src/?.lua", basePath.str);
			vm.AddPackagePath("%stransgressor/vst/compiler/?.lua", basePath.str);

			vm.Require("libs.base_synth_gui.script.init");
			vm.Require("transgressor.vst.gui.script.root");

			vm.LoadChunk(chunkStr, chunkStrLen);
			vm.GenerateProgramForHeader(&programSize, (void**)&program, &bytecodeLimitsSize, &bytecodeLimits);
			
			program->Unpack();
#endif
		}

		if (name)
		{
			instrumentChannel.push_back(atoi(channel->GetText()));
			controlChangeValues.push_back(vector<int>(128, -1));
		}
		else
		{
			instrumentChannel.push_back(-1);
			controlChangeValues.push_back(vector<int>(128, -1));
		}

		instrumentNum++;
	}

	// Second, create stack state vector
	vector<TrackState> trackStates;

	TiXmlElement *sequencerTrack = root.FirstChild("Tracks").FirstChild("SequencerTrack").ToElement();

	for (sequencerTrack; sequencerTrack; sequencerTrack=sequencerTrack->NextSiblingElement())
	{
		trackStates.push_back(TrackState());
	}

	// Third, get music data

	// ... loop over pattern sequence
	TiXmlElement *patternSequence = root.FirstChild("PatternSequence").FirstChild("SequenceEntries").FirstChild("SequenceEntry").ToElement();

	int patternTime = 0;

	for (patternSequence; patternSequence; patternSequence=patternSequence->NextSiblingElement())
	{
		int patternIndex = atoi(TiXmlHandle(patternSequence).FirstChild("Pattern").ToElement()->GetText());
		cout << "PATTERN " << patternIndex << endl;

		// Find pattern with correct index
		TiXmlElement *pattern = root.FirstChild("PatternPool").FirstChild("Patterns").Child(patternIndex).ToElement();

		int numLines = atoi(TiXmlHandle(pattern).FirstChild("NumberOfLines").ToElement()->GetText());
		numLinesInPattern.push_back(numLines);

		// Loop over tracks
		TiXmlElement *track = TiXmlHandle(pattern).FirstChild("Tracks").FirstChild("PatternTrack").ToElement();

		int trackIndex = 0;

		for (track; track; track=track->NextSiblingElement())
		{
			TrackState &trackState = trackStates[trackIndex];

			// Loop over lines (NOTE: Does not support polyphonic aftertouch)
			TiXmlElement *line = TiXmlHandle(track).FirstChild("Lines").FirstChild("Line").ToElement();

			cout << "TRACK" << trackIndex << endl;
			for (line; line; line=line->NextSiblingElement())
			{
				int lineTime = atoi(line->Attribute("index"));

				if (lineTime < numLines)
				{
					int time = patternTime + lineTime;

					TiXmlElement *noteColumn = TiXmlHandle(line).FirstChild("NoteColumns").FirstChild("NoteColumn").ToElement();
					TiXmlElement *lastNoteColumn = NULL;

					int noteColumnIndex = 0;

					// Loop over note columns
					for (noteColumn; noteColumn; noteColumn=noteColumn->NextSiblingElement())
					{
						NoteColumnState &noteColumnState = trackState.noteColumn[noteColumnIndex];

						lastNoteColumn = noteColumn;

						TiXmlElement *noteElement = TiXmlHandle(noteColumn).FirstChild("Note").ToElement();
						TiXmlElement *volumeElement = TiXmlHandle(noteColumn).FirstChild("Volume").ToElement(); 
						TiXmlElement *instrumentElement = TiXmlHandle(noteColumn).FirstChild("Instrument").ToElement();

						// Note on or off
						if (noteColumn && noteElement && noteElement->GetText()[0]!='-') 
						{
							int note = StrToNote(noteElement->GetText());

							// Note on
							if (note >= 0)
							{
								// Get note information
								int instrumentIndex = HexToInt(instrumentElement->GetText());
								int velocity = (volumeElement && volumeElement->GetText()[0]!='.') ? HexToInt(volumeElement->GetText()) : 0x7F;

								// If track is active, note off current note
								if (noteColumnState.IsActive())
								{
									noteColumnState.event->length = time - noteColumnState.event->time;
								}
									
								// Set note on
								ZRenoiseEvent* event = ZRenoiseEvent::MakeNoteOn(instrumentChannel[instrumentIndex], time, trackIndex, note, velocity);
								events.push_back(event);

								noteColumnState.SetActive(instrumentIndex, note);
								noteColumnState.event = event;
							}
							// Note off
							else
							{
								if (noteColumnState.IsActive())
								{
									// If track is active, note off current note
									noteColumnState.event->length = time - noteColumnState.event->time;

									// Update track state
									noteColumnState.SetInactive();
								}
							}
						}

						noteColumnIndex++;
					}

					
					// Parameter set
					// (We need the panning element from the last note column, and the values from the first effect column)
					TiXmlElement *effectColumn = TiXmlHandle(line).FirstChild("EffectColumns").FirstChild("EffectColumn").ToElement();

					TiXmlElement *valueElement = TiXmlHandle(effectColumn).FirstChild("Value").ToElement();
					TiXmlElement *numberElement = TiXmlHandle(effectColumn).FirstChild("Number").ToElement();
					
					TiXmlElement *panningElement = TiXmlHandle(lastNoteColumn).FirstChild("Panning").ToElement();
					TiXmlElement *instrumentElement = TiXmlHandle(lastNoteColumn).FirstChild("Instrument").ToElement();

					if (instrumentElement && panningElement && valueElement && numberElement && panningElement->GetText()[0]=='M' && panningElement->GetText()[1]=='0')
					{
						int instrumentIndex = HexToInt(instrumentElement->GetText());

						int number = HexToInt(numberElement->GetText());
						int value = HexToInt(valueElement->GetText());

						if (controlChangeValues[instrumentIndex][number] != value) // discard redundant changes
						{
							ZRenoiseEvent* event = ZRenoiseEvent::MakeControlChange(instrumentChannel[instrumentIndex], time, trackIndex, number, value);
							events.push_back(event);

							controlChangeValues[instrumentIndex][number] = value;
						}
					}

				}
			}

			trackIndex++;
		}

		patternTime += numLines;

		cout << numLines << endl;	
	}
	
	// Fourth, sort music data (inserted per track, not instrument, so may overlap)
	std::stable_sort(events.begin(), events.end(), SongEventComparator());

	// Fifth, get bpm
	TiXmlElement *bpmElement = root.FirstChild("GlobalSongData").FirstChild("BeatsPerMin").ToElement();
	bpm = atoi(bpmElement->GetText());

	// Sixth, get master track volume
	TiXmlElement *volumeElement = root.FirstChild("Tracks").FirstChild("SequencerMasterTrack").FirstChild("FilterDevices").FirstChild("Devices").FirstChild("SequencerMasterTrackDevice").FirstChild("PostVolume").FirstChild("Value").ToElement();
	masterTrackVolume = atof(volumeElement->GetText());
}

#include <set>

template <typename T>
T* VectorToBuffer(const std::vector<T>& vec)
{
	T* data = new T[vec.size];

	for (size_t i=0; i<vec.size(); i++)
		data[i] = vec[i];
}

#define VectorFieldToParameter(parameter, vector, field)\
	{\
		uint32_t i = 0;\
		decltype(exeSong->parameter) parameter##data = new remove_reference<decltype(*exeSong->parameter)>::type[vector.size()];\
		for (auto element : vector)\
		{\
			parameter##data[i] = element##field;\
			i++;\
		}\
		exeSong->parameter = parameter##data;\
	}

#define VectorToParameter(parameter, vector)\
	decltype(exeSong->parameter) parameter##data = new remove_reference<decltype(*exeSong->parameter)>::type[vector.size()];\
	for (size_t i=0; i<vector.size(); i++)\
		parameter##data[i] = vector[i];\
	exeSong->parameter = parameter##data;

ZExeSong* ZRenoiseSong::MakeExeSong()
{
	// Create list of unique note events
	std::set<UniqueNoteEvent, UniqueNoteEventCompare> uniqueNoteEvents;

	for (auto event : events)
	{
		UniqueNoteEvent uniqueEvent(*event);
		if (uniqueNoteEvents.find(uniqueEvent)==uniqueNoteEvents.end())			
		{
			uniqueNoteEvents.insert(uniqueEvent);
		}
	}

	// Set sorted index for unique note events (we cannot modify the contents of a set, so we create a new one)
	std::set<UniqueNoteEvent, UniqueNoteEventCompare> uniqueNoteEventsWithIndex;

	uint32_t sortedIndex = 0;

	for (auto uniqueEvent : uniqueNoteEvents)
	{
		UniqueNoteEvent updatedEvent = uniqueEvent;
		updatedEvent.index = sortedIndex++;
		uniqueNoteEventsWithIndex.insert(updatedEvent);
	}

	// Create list of unique control change events
	std::set<UniqueControlChangeEvent, UniqueControlChangeEventCompare> uniqueControlChangeEvents;

	for (auto event : events)
	{
		UniqueControlChangeEvent uniqueEvent(*event);
		if (uniqueControlChangeEvents.find(uniqueEvent)==uniqueControlChangeEvents.end())			
		{
			uniqueControlChangeEvents.insert(uniqueEvent);
		}
	}

	// Set sorted index for unique note events (we cannot modify the contents of a set, so we create a new one)
	std::set<UniqueControlChangeEvent, UniqueControlChangeEventCompare> uniqueControlChangeEventsWithIndex;

	sortedIndex = 0;

	for (auto uniqueEvent : uniqueControlChangeEvents)
	{
		UniqueControlChangeEvent updatedEvent = uniqueEvent;
		updatedEvent.index = sortedIndex++;
		uniqueControlChangeEventsWithIndex.insert(updatedEvent);
	}

	// Fill better event tables (not delta encoded yet)
	for (auto event : events)
	{
		if (event->type==ZRenoiseEvent::kTypeNote)
		{
			auto uniqueEvent = uniqueNoteEventsWithIndex.find(UniqueNoteEvent(*event));
			timedEvents.push_back(TimedEvent(event->instrument, event->time, uniqueEvent->index));
		}
		else if (event->type==ZRenoiseEvent::kTypeControlChange)
		{
			auto uniqueEvent = uniqueControlChangeEventsWithIndex.find(UniqueControlChangeEvent(*event));
			timedEvents.push_back(TimedEvent(event->instrument, event->time, uniqueEvent->index | ZExeSong::kEventCCMask));
		}
	}

	// Make num events per instrument vector
	std::vector<int> numEventsPerInstrumentVector(ZExeSong::kNumInstruments, 0);

	for (auto event : events)
		numEventsPerInstrumentVector[event->instrument]++;

	int offset = 0;
	std::vector<int> instrumentEventOffset(ZExeSong::kNumInstruments, 0);

	for (uint32_t i=0; i<ZExeSong::kNumInstruments; i++)
	{
		instrumentEventOffset[i] = offset;
		offset += numEventsPerInstrumentVector[i];
	}

	ZExeSong* exeSong = new ZExeSong();

	// Delta encode time information
	for (uint32_t i=0; i<ZExeSong::kNumInstruments; i++)
	{
		int lastLine = 0;

		for (int e=instrumentEventOffset[i]; e<instrumentEventOffset[i]+numEventsPerInstrumentVector[i]; e++)
		{
			auto curLine = timedEvents[e].time;
			timedEvents[e].time -= lastLine;
			lastLine = curLine;
		}
	}

	// Build struct
	exeSong->bpm                          = bpm;
	exeSong->masterTrackVolume            = masterTrackVolume;

	exeSong->numInstruments               = ZExeSong::kNumInstruments;
	exeSong->numLines                     = GetNumLines();
	exeSong->numUniqueNoteEvents          = uniqueNoteEventsWithIndex.size();
	exeSong->numUniqueControlChangeEvents = uniqueControlChangeEventsWithIndex.size();

	VectorToParameter(numEventsPerInstrument, numEventsPerInstrumentVector);

	VectorFieldToParameter(eventTime, timedEvents, .time);
	VectorFieldToParameter(eventId, timedEvents, .eventId);

	VectorFieldToParameter(uneNote, uniqueNoteEventsWithIndex, .note);
	VectorFieldToParameter(uneVelocity, uniqueNoteEventsWithIndex, .velocity);
	VectorFieldToParameter(uneLength, uniqueNoteEventsWithIndex, .length);

	VectorFieldToParameter(ucceNumber, uniqueControlChangeEventsWithIndex, .number);
	VectorFieldToParameter(ucceValue, uniqueControlChangeEventsWithIndex, .value);

	return exeSong;
}
void WriteDataToHeader(const char* path, uint8_t* data, size_t dataSize)
{
	// Open file
	ofstream file;
	file.open(path);

	file << "uint8_t data[] = \n";
	file << "{\n";

	for (uint32_t i=0; i<dataSize; i++)
		file << "0x" << hex << setw(2) << setfill('0') << uint32_t(data[i]) << ", ";

	file << "\n};\n";

	// Close file
	file.close();
}

int ZRenoiseSong::GetNumLines()
{
	int numLines = 0;

	for (int linesInPattern : numLinesInPattern)
		numLines += linesInPattern;

	return numLines;
}

void ZRenoiseEvent::SetNoteOn(int instrument, int time, int track, int note, int velocity)
{
	type = kTypeNote;
	this->time=time;
	this->note=note;
	this->velocity=velocity;
	this->length=-1;
	this->track=track;
	this->instrument=instrument;
}

void ZRenoiseEvent::SetControlChange(int instrument, int time, int track, int number, int value)
{
	this->type=kTypeControlChange;
	this->time=time;
	this->number=number;
	this->value=value;
	this->track=track;
	this->instrument=instrument;
}
