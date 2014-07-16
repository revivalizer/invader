#pragma once

#pragma warning(push)
#pragma warning(disable : 4668)
#include "public.sdk/source/vst2.x/audioeffectx.h"
#pragma warning(pop)

class LuaVM;
class ZSynth;
class ZResampler2x;

typedef std::deque<VstMidiEvent> event_queue_t;

class VSTInstrument : public AudioEffectX, public align16
{
public:
	VSTInstrument(audioMasterCallback audioMaster);
	~VSTInstrument();

	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual VstInt32 processEvents (VstEvents* events);

	virtual void setSampleRate (float sampleRate);

	virtual void resume();
	virtual void suspend();

	virtual bool getOutputProperties (VstInt32 index, VstPinProperties* properties);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
	virtual VstInt32 canDo (char* text);

	virtual VstInt32 getNumMidiInputChannels ();
	virtual VstInt32 getNumMidiOutputChannels ();

	VstInt32 getChunk(void **data, bool	isPreset = false);
	VstInt32 setChunk(void *data, VstInt32 byteSize, bool isPreset = false);

private:
	//static const int kMaxNameLength = 255;
	//char programName[kMaxNameLength + 1];

	//LuaVM* vm;

	ZMutex eventQueueMutex;
	event_queue_t eventQueue;

	ZCircularBuffer<16384> sampleBuffer;
	ZBlockBuffer1x downsampleBuffer;
	invader::ZResampler2x* downsampler;

	invader::ZSynth* synth;

//	VSTEditor* vstEditor;
};