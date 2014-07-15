#include "pch.h"
#include "delay.h"

ZDelay::ZDelay(nodetype_t type) : ZSynthNode(type)
{
	buffer.Reset();
}

void ZDelay::Process(ZVirtualMachine* vm, argument_t argument)
{
	argument;
	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	static const double sqh = zsqrtd(0.5);

	static const sample_t echoMatrix[4][4] =  // Modes, inputs (left input, right input, left feedback, right feedback), channel
	{
		{ sample_t(1.0, 0.0), sample_t(0.0, 1.0), sample_t(1.0, 0.0), sample_t(0.0, 1.0) }, // unity
		{ sample_t(0.0, 1.0), sample_t(1.0, 0.0), sample_t(0.0, 1.0), sample_t(1.0, 0.0) }, // invert
		{ sample_t(sqh, 0.0), sample_t(sqh, 0.0), sample_t(0.0, 1.0), sample_t(1.0, 0.0) }, // pingpong left
		{ sample_t(0.0, sqh), sample_t(0.0, sqh), sample_t(0.0, 1.0), sample_t(1.0, 0.0) }, // pingpong left
	};

	ZBlockBufferInternal block;
	block.Reset();

	uint32_t mode  = zclamp(zifloord(vm->stack[-5]), 0, 3);
	double   time  = vm->stack[-4];
	sample_t decay = dbToGain(vm->stack[-3] * time);
	sample_t dry   = sample_t(dbToGain(vm->stack[-2]));
	sample_t wet   = sample_t(dbToGain(vm->stack[-1]));

	uint32_t sampleDelay = zifloord(time*kSampleRate);

	mode; time; decay; dry; wet;

	vm->stack -= 5;

	ZBlockBufferInternal& in = synthvm->blockStack->Pop();

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		sample_t input = in.samples[i];
		sample_t output = buffer.ReadOffsetSample(sampleDelay);

		buffer.PutSample(
			(echoMatrix[mode][0] * input.d[1] +  // note that they are switched due to endianess!
			 echoMatrix[mode][1] * input.d[0] + 
			 echoMatrix[mode][2] * output.d[1] + 
			 echoMatrix[mode][3] * output.d[0])
			 * decay
		);

		block.samples[i] = input*dry + output*wet;
	}

	synthvm->blockStack->Push(block);
}