#pragma once

void WriteDataToHeader(const char* path, uint8_t* data, size_t dataSize);


class ZRenoiseEvent
{
public:
	enum
	{
		kTypeNote = 0,
		kTypeControlChange,
	};

	static ZRenoiseEvent* MakeNoteOn(int instrument, int time, int track, int note, int velocity)
	{
		ZRenoiseEvent* event = new ZRenoiseEvent;
		event->SetNoteOn(instrument, time, track, note, velocity);
		return event;
	}

	static ZRenoiseEvent* MakeControlChange(int instrument, int time, int track, int number, int value)
	{
		ZRenoiseEvent* event = new ZRenoiseEvent;
		event->SetControlChange(instrument, time, track, number, value);
		return event;
	}

	void SetNoteOn(int instrument, int time, int track, int note, int velocity);
	void SetControlChange(int instrument, int time, int track, int control, int value);

	int type;
	int time;
	int note; // midi note
	int velocity;
	int length; // time before note off, in lines
	int number;
	int value;
	int track; // used for sorting
	int instrument;
};

struct UniqueNoteEvent
{
	UniqueNoteEvent(const ZRenoiseEvent& event)
		: note(event.note)
		, velocity(event.velocity)
		, length(event.length)
	{

	}

	int note;
	int velocity;
	int length;

	int index;
};

struct UniqueNoteEventCompare
{
	bool operator()(const UniqueNoteEvent& s1, const UniqueNoteEvent& s2)
	{
		int s1n = (s1.note*1024 + s1.length)*256 + s1.velocity;
		int s2n = (s2.note*1024 + s2.length)*256 + s2.velocity;

		return s1n < s2n;
	}
};

struct UniqueControlChangeEvent
{
	UniqueControlChangeEvent(const ZRenoiseEvent& event)
		: number(event.number)
		, value(event.value)
	{

	}

	int number;
	int value;

	int index;
};

struct UniqueControlChangeEventCompare
{
	bool operator()(const UniqueControlChangeEvent& s1, const UniqueControlChangeEvent& s2)
	{
		int s1n = s1.number*256 + s1.value;
		int s2n = s2.number*256 + s2.value;

		return s1n < s2n;
	}
};


struct TimedEvent
{
	TimedEvent(int instrument, int time, int eventId)
		: instrument(instrument)
		, time(time)
		, eventId(eventId) {}
	int instrument;
	int time;
	int eventId;
};




struct SongEventComparator
{
	bool operator() (ZRenoiseEvent* const & a, ZRenoiseEvent* const & b)	
	{
		int aVal[4] = {a->instrument, a->time, a->track, a->type}; // this only works because of the type definition, and is a little bit dangerous
		int bVal[4] = {b->instrument, b->time, b->track, b->type};

		// Lexicographical comparison
		for (int i=0; i<4; i++)
		{
			if (aVal[i] < bVal[i])
				return true;
			else if (aVal[i] > bVal[i])
				return false;
		}

		// If we get to here, they're equal
		return false;
	}
};

struct ZExeSong;

class ZRenoiseSong
{
	public:
		ZRenoiseSong(void);
		~ZRenoiseSong(void);

		int Read(const char* path);
		void WriteHeader(const char* path);
		//void WriteHeader2(const char* path);

		std::vector<int> numLinesInPattern;

		std::vector<ZRenoiseEvent*> events;
		std::vector<TimedEvent> timedEvents;

		int bpm;
		double masterTrackVolume;

		int GetNumLines();

		ZExeSong* MakeExeSong();

private:
		int LoadRenoiseFile(const char* path, char** data, int* dataLength);
		void MakeUniformLineEndings(char* buf, int length);
		void ConvertXMLToSong(TiXmlDocument& doc);

		char* xmlData;
		int xmlDataLength;

};

