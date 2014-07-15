#include "pch.h"
#include "reverb.h"

// Based on this: http://www.gearslutz.com/board/geekslutz-forum/380233-reverb-subculture.html

ZReverb::ZReverb(nodetype_t type) : ZSynthNode(type)
	, diffusorLeft (20.0*0.7*0.61803*1.05, 0.35, 20.0*0.7*(1.0-0.61803)*1.05, -0.45, 2.5*1.05, 0.60)
	, diffusorRight(20.0*0.7*0.61803*0.95, -0.35, 20.0*0.7*(1.0-0.61803)*0.95, 0.45, 2.5*0.95, -0.60)
{
	for (uint32_t i=0; i<kNumLoops; i++)
		loop[i].Reset();

	ZRandom r(872134);

	for (uint32_t i=0; i<kNumLoops; i++)
		for (uint32_t j=0; j<kTapsPerLoop; j++)
			grain[i][j].init(r.NextUniformInt());

	inputFilter1.SetCutoff(9000);
	inputFilter2.SetCutoff(9000);

	preDelayBuffer.Reset();
}

void ZReverb::Process(ZVirtualMachine* vm, argument_t argument)
{
	argument;

	ZSynthVirtualMachine* synthvm = static_cast<ZSynthVirtualMachine*>(vm);

	ZBlockBufferInternal block;
	block.Reset();

	/*sample_t tapgain1    = sample_t(dbToGain(vm->stack[-24]));
	sample_t tapgain2    = sample_t(dbToGain(vm->stack[-23]));
	sample_t tapgain3    = sample_t(dbToGain(vm->stack[-22]));

	sample_t tapgain4    = sample_t(dbToGain(vm->stack[-21]));
	sample_t tapgain5    = sample_t(dbToGain(vm->stack[-20]));
	sample_t tapgain6    = sample_t(dbToGain(vm->stack[-19]));

	sample_t tapgain7    = sample_t(dbToGain(vm->stack[-18]));
	sample_t tapgain8    = sample_t(dbToGain(vm->stack[-17]));
	sample_t tapgain9    = sample_t(dbToGain(vm->stack[-16]));*/

	double tapgains[] = { -4.5, -6.5, -9.0, -4.0, -7.0, -8.0, -8.0, -11.0, -14.0};
	sample_t tapgains_s[9];

	for (uint32_t i=0; i<9; i++)
		tapgains_s[i] = sample_t(dbToGain(tapgains[i]));
 
	/*double tap1   = vm->stack[-15] / 1000.0 * kSampleRate;
	double tap2   = vm->stack[-14] / 1000.0 * kSampleRate;
	double tap3   = vm->stack[-13] / 1000.0 * kSampleRate;

	double tap4   = vm->stack[-12] / 1000.0 * kSampleRate;
	double tap5   = vm->stack[-11] / 1000.0 * kSampleRate;
	double tap6   = vm->stack[-10] / 1000.0 * kSampleRate;

	double tap7   = vm->stack[-9] / 1000.0 * kSampleRate;
	double tap8   = vm->stack[-8] / 1000.0 * kSampleRate;
	double tap9   = vm->stack[-7] / 1000.0 * kSampleRate;*/

	double room_size = vm->stack[-7];

	double delay1   = vm->stack[-10] / 1000.0 * kSampleRate * room_size/330.0;
	double delay2   = vm->stack[-9] / 1000.0 * kSampleRate * room_size/330.0;
	double delay3   = vm->stack[-8] / 1000.0 * kSampleRate * room_size/330.0;


	double tap1 = delay1 * 1.0/4.0;
	double tap2 = delay1 * 2.0/4.0;
	double tap3 = delay1 * 3.0/4.0;

	double tap4 = delay2 * 1.0/4.0;
	double tap5 = delay2 * 2.0/4.0;
	double tap6 = delay2 * 3.0/4.0;

	double tap7 = delay3 * 1.0/4.0;
	double tap8 = delay3 * 2.0/4.0;
	double tap9 = delay3 * 3.0/4.0;


	/*
	sample_t gain1    = sample_t(dbToGain(vm->stack[-3]));
	sample_t gain2    = sample_t(dbToGain(vm->stack[-2]));
	sample_t gain3    = sample_t(dbToGain(vm->stack[-1]));
	*/

	double decay = vm->stack[-6]; // we are not using the other two params

	sample_t gain1    = sample_t(dbToGain(decay / (kSampleRate / delay1)));
	sample_t gain2    = sample_t(dbToGain(decay / (kSampleRate / delay2)));
	sample_t gain3    = sample_t(dbToGain(decay / (kSampleRate / delay3)));

	sample_t dry    = sample_t(dbToGain(vm->stack[-5]));
	sample_t wet    = sample_t(dbToGain(vm->stack[-4]));

	double spread = vm->stack[-3] / 100.0;
	double predelay = zclamp(vm->stack[-2] / 1000.0 * kSampleRate, 1.0, 32767);
	double modRate = vm->stack[-1];

	vm->stack -= 10;

	ZBlockBufferInternal& in = synthvm->blockStack->Pop();

	delay1, delay2, delay3, gain1, gain2, gain3;

	sample_t panLeft = ComputePan(-3.0, 0.5 - spread);
	sample_t panRight = ComputePan(-3.0, 0.5 + spread);

	double delays[] = {delay1, delay2, delay3};
	sample_t gains[] = {gain1, gain2, gain3};

	ZBlockBufferInternal outLeft;
	outLeft.Reset();
	ZBlockBufferInternal outRight;
	outRight.Reset();

	for (uint32_t i=0; i<block.numSamples; i++)
	{
		sample_t input = in.samples[i];
		preDelayBuffer.PutSample(input);

		sample_t inputDelayed = preDelayBuffer.ReadOffsetSample(zifloord(predelay));

		sample_t inputFiltered = inputFilter1.Lowpass(inputFilter2.Lowpass(inputDelayed));

		//for (uint32_t j=0; j<kNumLoops; j++)
		loop[0].PutSample(inputFiltered + (loop[2].ReadOffsetSample(zifloord(delays[2]))*gains[0])); // no gain modulation
		loop[1].PutSample(inputFiltered + (loop[0].ReadOffsetSample(zifloord(delays[0]))*gains[1])); // no gain modulation
		loop[2].PutSample(inputFiltered + (loop[1].ReadOffsetSample(zifloord(delays[1]))*gains[2])); // no gain modulation
		//loop[0].PutSample(input + diffusor0.Process(loop[2].ReadOffsetSample(zifloord(delays[0]))*gains[0])); // no gain modulation
		//loop[1].PutSample(input + diffusor1.Process(loop[0].ReadOffsetSample(zifloord(delays[1]))*gains[1])); // no gain modulation
		//loop[2].PutSample(input + diffusor2.Process(loop[1].ReadOffsetSample(zifloord(delays[2]))*gains[2])); // no gain modulation
		//loop[1].PutSample((input + loop[1].ReadOffsetSample(zifloord(delays[1])))*gains[1] + loop[0].ReadOffsetSample(zifloord(delays[0]))*gains[0]*gains[1]); // no gain modulation
		//loop[2].PutSample((input + loop[2].ReadOffsetSample(zifloord(delays[2])))*gains[2] + loop[1].ReadOffsetSample(zifloord(delays[1]))*gains[1]*gains[2]); // no gain modulation
		//loop2.PutSample((input + loop2.ReadOffsetSample(zifloord(delay2)))*gain2);
		//loop3.PutSample((input + loop3.ReadOffsetSample(zifloord(delay3)))*gain3);
		
		//sample_t outLeft = sample_t::zero();
		//sample_t outRight = sample_t::zero();
	}

	for (uint32_t i=0; i<block.numSamples; i++)
		outLeft.samples[i]   += grain[0][0].Update(tap1, delays[0], &loop[0], modRate)*tapgains_s[0];
	for (uint32_t i=0; i<block.numSamples; i++)
		outRight.samples[i]  += grain[0][1].Update(tap2, delays[0], &loop[0], modRate)*tapgains_s[1];
	for (uint32_t i=0; i<block.numSamples; i++)
		outLeft.samples[i]   += grain[0][2].Update(tap3, delays[0], &loop[0], modRate)*tapgains_s[2];

	for (uint32_t i=0; i<block.numSamples; i++)
		outLeft.samples[i]   += grain[1][0].Update(tap4, delays[1], &loop[1], modRate)*tapgains_s[3];
	for (uint32_t i=0; i<block.numSamples; i++)
		outRight.samples[i]  += grain[1][1].Update(tap5, delays[1], &loop[1], modRate)*tapgains_s[4];
	for (uint32_t i=0; i<block.numSamples; i++)
		outRight.samples[i]  += grain[1][2].Update(tap6, delays[1], &loop[1], modRate)*tapgains_s[5];

	for (uint32_t i=0; i<block.numSamples; i++)
		outRight.samples[i]  += grain[2][0].Update(tap7, delays[2], &loop[2], modRate)*tapgains_s[6];
	for (uint32_t i=0; i<block.numSamples; i++)
		outLeft.samples[i]   += grain[2][1].Update(tap8, delays[2], &loop[2], modRate)*tapgains_s[7];
	for (uint32_t i=0; i<block.numSamples; i++)
		outLeft.samples[i]   += grain[2][2].Update(tap9, delays[2], &loop[2], modRate)*tapgains_s[8];

		/*outLeft  += loop[0].ReadOffsetSample(zifloord(tap1))*tapgains_s[0];
		outRight += loop[0].ReadOffsetSample(zifloord(tap2))*tapgains_s[1];
		outLeft  += loop[0].ReadOffsetSample(zifloord(tap3))*tapgains_s[2];

		outLeft  += loop[1].ReadOffsetSample(zifloord(tap4))*tapgains_s[3];
		outRight += loop[1].ReadOffsetSample(zifloord(tap5))*tapgains_s[4];
		outRight += loop[1].ReadOffsetSample(zifloord(tap6))*tapgains_s[5];

		outRight += loop[2].ReadOffsetSample(zifloord(tap7))*tapgains_s[6];
		outLeft  += loop[2].ReadOffsetSample(zifloord(tap8))*tapgains_s[7];
		outLeft  += loop[2].ReadOffsetSample(zifloord(tap9))*tapgains_s[8];
		*/

	for (uint32_t i=0; i<block.numSamples; i++)
		block.samples[i] = dry*in.samples[i] + wet*(diffusorLeft.Process(outLeft.samples[i]*panLeft) + diffusorRight.Process(outRight.samples[i]*panRight));
		//block.samples[i] = outLeft*panLeft + outRight*panRight;

	synthvm->blockStack->Push(block);
}
