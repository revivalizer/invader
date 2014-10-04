#include "pch.h"

namespace invader {

ZVirtualMachine::ZVirtualMachine(ZVMProgram* program, ZVMStack* stack, ZVMStorage* globalStorage)
	: nodeInstances(nullptr)
	, stack(stack)
	, program(program)
	, globalStorage(globalStorage)
{
//#error must init variable array
}

ZVirtualMachine::~ZVirtualMachine(void)
{
	if (nodeInstances)
	{
		for (uint32_t i=0; i<program->bytecodeSize; i++)
		{
			if (nodeInstances[i])
			{
				delete nodeInstances[i];
				nodeInstances[i] = nullptr;
			}
		}

		delete nodeInstances;
		nodeInstances = nullptr;
	}

	// Only one global storage exists, freed by synth
	globalStorage = nullptr;

	if (stack)
	{
		zalignedfree((void*)stack->mem);
		stack->mem = 0;

		delete stack;
		stack = nullptr;
	}
}

// TODO: FIGURE OUT WHY THIS IS NECCESARY
#pragma warning(disable: 4127)

#if 0
	#define trace(...) _zmsg(__VA_ARGS__)
#else
	#define trace(...) void(0)
#endif 

void ZVirtualMachine::Run(opcode_t start_address, ZVMProgram* program)
{
	opcode_t* ip = &(program->bytecode[start_address]);

	while (true)
	{
		opcode_t opcode = *ip++;

		if (opcode & kOpcodeMaskIsNode)
		{
			trace("%04x call node, type %x", ip-program->bytecode-1, nodeInstances[ip-program->bytecode-1]);

			auto node = nodeInstances[ip-program->bytecode-1]; 

			if (!node)
			{
				node = nodeInstances[ip-program->bytecode-1] = CreateNodeFromOpcode(ip[-1]);

				if (voice)
					node->NoteOn(voice->pitch, voice->note, voice->velocity, voice->deltaSamples);
			}

			node->Process(this);
		}
		else
		{
			switch (opcode)
			{
				case kOpReturn:
					trace("0x%04x halt", ip-program->bytecode-1);
					return;
				case kOpPush | kOpTypeNum:
					trace("0x%04x push constant num id %d, value: %f", ip-program->bytecode-1, *ip, program->constants[*ip]);
					stack->Push<num_t>(program->constants[*ip++]);
					break;
				/*case kOpPop:
					numstack--;
					ip+=1;
					break;
				case kOpDec:
					numstack[-1]--;
					ip+=1;
					break;
				case kOpClone:
					numstack[0] = numstack[-1];
					numstack++;
					ip+=1;
					break;*/
				case kOpPushGlobal | kOpTypeNum:
					trace("0x%04x push global num, address: 0x%04x", ip-program->bytecode-1, *ip);
					stack->Push(globalStorage->Load<num_t>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeSample:
					trace("0x%04x push global sample, address: 0x%04x", ip-program->bytecode-1, *ip);
					stack->Push(globalStorage->Load<ZBlockBufferInternal>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeSpectrum:
					trace("0x%04x push global spectrum, address: 0x%04x", ip-program->bytecode-1, *ip);
					stack->Push(globalStorage->Load<ZRealSpectrum>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeWavetable:
					trace("0x%04x push global wavetable, address: 0x%04x", ip-program->bytecode-1, *ip);
					stack->Push(globalStorage->Load<ZWaveformWavetable<>*>(*ip++));
					break;
				case kOpPopGlobal | kOpTypeNum:
					trace("0x%04x pop global num, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Store<num_t>(*ip++, stack->Pop<num_t>());
					break;
				case kOpPopGlobal | kOpTypeSample:
					trace("0x%04x pop global sample, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Store<ZBlockBufferInternal>(*ip++, stack->Pop<ZBlockBufferInternal>());
					break;
				case kOpPopGlobal | kOpTypeSpectrum:
					trace("0x%04x pop global spectrum, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Store<ZRealSpectrum>(*ip++, stack->Pop<ZRealSpectrum>());
					break;
				case kOpPopGlobal | kOpTypeWavetable:
					trace("0x%04x pop global wavetable, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Store<ZWaveformWavetable<>*>(*ip++, stack->Pop<ZWaveformWavetable<>*>());
					break;
				case kOpResetGlobal | kOpTypeNum:
					trace("0x%04x reet global num, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Reset<num_t>(*ip++);
					break;
				case kOpResetGlobal | kOpTypeSample:
					trace("0x%04x reet global sample, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Reset<ZBlockBufferInternal>(*ip++);
					break;
				case kOpResetGlobal | kOpTypeSpectrum:
					trace("0x%04x reet global spectrum, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Reset<ZRealSpectrum>(*ip++);
					break;
/*				case kOpResetGlobal | kOpTypeWavetable:
					trace("0x%04x reet global wavetable, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Reset<ZWaveformWavetable<>>(*ip++);
					break;
				*/
				/*case kOpJump:
					ip = program->labels[argument];
					break;
				case kOpJumpEqual:
					if (numStack[-1]==0.)
						ip = program->labels[argument];
					else
						ip += 2;
					numStack--;
					break;
				case kOpJumpNotEqual:
					if (numStack[-1]!=0.)
						ip = program->labels[argument];
					else
						ip += 2;
					numStack--;
					break;*/

				case kOpAdd | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 + op2);
						trace("0x%04x add num x num, %f + %f = %f", ip-program->bytecode-1, op1, op2, op1 + op2);
						break;
					}

				case kOpAdd | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->Pop<ZBlockBufferInternal>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] += op2.samples[i];
						stack->Push(op1);
						trace("0x%04x add sample x sample", ip-program->bytecode-1);
						break;
					}

				case kOpAdd | 2: // sample x num
					{
						num_t op2 = stack->Pop<num_t>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] += s;
						stack->Push(op1);
						trace("0x%04x add sample x num (%f)", ip-program->bytecode-1, op2);
						break;
					}

				case kOpSubtract | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 - op2);
						trace("0x%04x subtract num x num, %f + %f = %f", ip-program->bytecode-1, op1, op2, op1 + op2);
						break;
					}

				case kOpSubtract | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->Pop<ZBlockBufferInternal>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] -= op2.samples[i];
						stack->Push(op1);
						trace("0x%04x subtract sample x sample", ip-program->bytecode-1);
						break;
					}

				case kOpSubtract | 2: // sample x num
					{
						num_t op2 = stack->Pop<num_t>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] -= s;
						stack->Push(op1);
						trace("0x%04x subtract sample x num (%f)", ip-program->bytecode-1, op2);
						break;
					}

				case kOpMultiply | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 * op2);
						trace("0x%04x mul num x num, %f * %f = %f", ip-program->bytecode-1, op1, op2, op1 * op2);
						break;
					}

				case kOpMultiply | 1: // sample x num
					{
						num_t op2 = stack->Pop<num_t>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] *= s;
						stack->Push(op1);
						trace("0x%04x mul sample x num (%f)", ip-program->bytecode-1, op2);
						break;
					}

				case kOpDivide | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 / op2);
						trace("0x%04x div num x num, %f / %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpDivide | 1: // sample x num
					{
						num_t op2 = stack->Pop<num_t>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] /= s;
						stack->Push(op1);
						trace("0x%04x div sample x num (%f)", ip-program->bytecode-1, stack->Top<num_t>());
						break;
					}

				case kOpModulo: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(zfmodd(op1, op2));
						trace("0x%04x mod num x num, %f %% %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 == op2) ? 1. : 0.);
						trace("0x%04x equal num x num, %f == %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpNotEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 != op2) ? 1. : 0.);
						trace("0x%04x not equal num x num, %f != %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLessThan: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 < op2) ? 1. : 0.);
						trace("0x%04x less than num x num, %f < %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLessThanOrEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 <= op2) ? 1. : 0.);
						trace("0x%04x less than or equal num x num, %f <= %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpGreaterThan: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 > op2) ? 1. : 0.);
						trace("0x%04x greater than num x num, %f > %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpGreaterThanOrEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 >= op2) ? 1. : 0.);
						trace("0x%04x greater than or equal num x num, %f >= %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLogicalAnd: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1!=0. && op2!=0.) ? 1. : 0.);
						trace("0x%04x and num x num, %f && %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLogicalOr: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1!=0. || op2!=0.) ? 1. : 0.);
						trace("0x%04x or num x num, %f || %f = %f", ip-program->bytecode-1, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpNot: // num
					{
						num_t op = stack->Pop<num_t>();
						stack->Push((op==0.) ? 1. : 0.);
						trace("0x%04x not num, !%f = %f", ip-program->bytecode-1, op, stack->Top<num_t>());
						break;
					}

				case kOpPlus: // num | sample
					{
						// do nothing
						break;
					}

				case kOpMinus | 0: // num
					{
						num_t op = stack->Pop<num_t>();
						stack->Push(-op);
						trace("0x%04x minus num, -%f = %f", ip-program->bytecode-1, op, stack->Top<num_t>());
						break;
					}

				case kOpMinus | 1: // num
					{
						ZBlockBufferInternal& op = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op.numSamples; i++)
							op.samples[i] *= sample_t(-1.0); // TODO: Use constant!
						stack->Push(op);
						trace("0x%04x minus sample", ip-program->bytecode-1);
						break;
					}

				case kOpCallFunc:
					{
						switch (*ip++)
						{
							case 1: // osc()
								{
									ZBlockBufferInternal out;

									for (double i=0.0; i<out.numSamples; i++)
									{
										double time = voice->timeSinceNoteOn + i/kSampleRate;
										out.samples[zifloord(i)] = sample_t(zsind(time*pitchToFrequency(voice->pitch)*kM_PI2));
									}

									stack->Push(out);
									trace("0x%04x osc()", ip-program->bytecode-1);
									break;
								}

							case 2: // map_midi_channel(channel, instrument)
								{
									auto instrument = uint32_t(stack->Pop<num_t>());
									auto channel    = uint32_t(stack->Pop<num_t>());
									synth->midiChannelToInstrumentMap[channel] = synth->GetInstrument(instrument);
									trace("0x%04x map_midi_channel(%f, %f)", ip-program->bytecode-1, instrument, channel);
									break;
								}

							case 3: // spectrum()
								{
									ZRealSpectrum spec;
									for (uint32_t i=0; i<spec.size; i++)
										spec.data[i] = complex_t(0.0);
									stack->Push(spec);
									trace("0x%04x spectrum()", ip-program->bytecode-1);
									break;
								}

							case 0xB01: // spectrum.toWavetable
								{
									ZRealSpectrum& spectrum = stack->Pop<ZRealSpectrum>();
									auto wavetable = new ZWaveformWavetable<>(spectrum);
									stack->Push(wavetable);
									trace("0x%04x spectrum.toWavetable()", ip-program->bytecode-1);
									break;
								}

							case 0xB03: // spectrum.addSine(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = uint32_t(zifloord(stack->Pop<num_t>()));

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();

									if (harmonic>0 && harmonic<spec.size)
										spec.data[harmonic] = complex_t(gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addSine(%d, %f)", ip-program->bytecode-1, harmonic, gain);
									break;
								}

							case 0xB04: // spectrum.addSaw(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zifloord(stack->Pop<num_t>()));

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=zmax(harmonic, 1); i<spec.size; i++)
										spec.data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addSaw(%d, %f)", ip-program->bytecode-1, harmonic, gain);
									break;
								}

							case 0xB05: // spectrum.addSquare(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zifloord(stack->Pop<num_t>()));

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=zmax(harmonic, 1); i<spec.size; i+=2)	
										spec.data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addSaw(%d, %f)", ip-program->bytecode-1, harmonic, gain);
									break;
								}

							case 0xB06: // spectrum.addTriangle(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zifloord(stack->Pop<num_t>()));

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=zmax(harmonic, 1); i<spec.size; i+=2)	
									{
										double n = double(i+1-harmonic);
										double sign = zfmodd((n+1)/2, 2)*2.0-1.0; // alternating sign for harmonics
										spec.data[i] = complex_t(sign / (n*n) * gain);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.addTriangle(%d, %f)", ip-program->bytecode-1, harmonic, gain);
									break;
								}

							case 0xB07: // spectrum.addThirds(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zifloord(stack->Pop<num_t>()));

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=zmax(harmonic, 1); i<spec.size; i+=3)	
										spec.data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addThirds(%d, %f)", ip-program->bytecode-1, harmonic, gain);
									break;
								}

							case 0xB08: // spectrum.addWhite(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zifloord(stack->Pop<num_t>()));

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=zmax(harmonic, 1); i<spec.size; i+=1)	
										spec.data[i] = complex_t(gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addWhite(%d, %f)", ip-program->bytecode-1, harmonic, gain);
									break;
								}

							case 0xB10: // spectrum.mulWhiteNoise(num seed)
								{
									auto seed = uint32_t(zifloord(stack->Pop<num_t>()));

									ZRandom r(seed);

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=1; i<spec.size; i++)	
										spec.data[i] *= r.NextUniformDouble();

									stack->Push(spec);
									trace("0x%04x spectrum.mulWhiteNoise(%d)", ip-program->bytecode-1, seed);
									break;
								}

							case 0xB11: // spectrum.mulWhiteNoiseDB(num seed, num dB)
								{
									auto db = stack->Pop<num_t>();
									auto seed = uint32_t(zifloord(stack->Pop<num_t>()));

									ZRandom r(seed);

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=1; i<spec.size; i++)	
										spec.data[i] *= dbToGain(r.NextUniformDouble()*db);

									stack->Push(spec);
									trace("0x%04x spectrum.mulWhiteNoiseDB(%d, %f)", ip-program->bytecode-1, seed, gain);
									break;
								}

							case 0xB12: // spectrum.spectrumNoise(num seed, num wavelength, num dB)
								{
									auto db = stack->Pop<num_t>();
									auto wavelength = stack->Pop<num_t>();
									auto seed = uint32_t(zifloord(stack->Pop<num_t>()));

									ZRandom r(seed);

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (double i=wavelength/2; i<spec.size+wavelength/2; i+=wavelength)	
										spec.ApplyPeak(i, wavelength/2.0, r.NextGaussianDouble()*db);
										// this is a liottle different from quiver
											// quiver -> amplitude passed as gain, not db
												// needs test


									stack->Push(spec);
									trace("0x%04x spectrum.spectrumNoise(%d, %d, %f, %f)", ip-program->bytecode-1, seed, wavelength, db);
									break;
								}

							case 0xB20: // spectrum.comb(num phase, num frequency)
								{
									auto freq = stack->Pop<num_t>();
									auto phaseOffset = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=1; i<spec.size; i++)	
									{
										double phase = double(i) * freq / spec.size - phaseOffset;
										phase = zfmodd(phase*kM_PI, kM_PI);
										spec.data[i] *= zsind(phase);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.comb(%d, %f, %f)", ip-program->bytecode-1, phase, frequency);
									break;
								}

							case 0xB21: // spectrum.inverseComb(num phase, num frequency)
								{
									auto freq = stack->Pop<num_t>();
									auto phaseOffset = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (int32_t i=1; i<spec.size; i++)	
									{
										double phase = double(i) * freq / spec.size - phaseOffset;
										phase = zfmodd(phase*kM_PI, kM_PI);
										spec.data[i] *= 1.0 - zsind(phase);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.inverseComb(%d, %f, %f)", ip-program->bytecode-1, phase, frequency);
									break;
								}

							case 0xB30: // spectrum.lowpass(num cutoff, num dbPerOctave)
								{
									auto dbPerOctave = stack->Pop<num_t>();
									auto cutoff      = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									spec.ApplyLowpass(cutoff, dbPerOctave);

									stack->Push(spec);
									trace("0x%04x spectrum.lowpass(%d, %f, %f)", ip-program->bytecode-1, cutoff, dbPerOctave);
									break;
								}

							case 0xB31: // spectrum.highpass(num cutoff, num dbPerOctave)
								{
									auto dbPerOctave = stack->Pop<num_t>();
									auto cutoff      = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									spec.ApplyHighpass(cutoff, dbPerOctave);

									stack->Push(spec);
									trace("0x%04x spectrum.highpass(%d, %f, %f)", ip-program->bytecode-1, cutoff, dbPerOctave);
									break;
								}

							case 0xB32: // spectrum.bandpass(num lowCutoff, num highCutoff, num dbPerOctave)
								{
									auto dbPerOctave   = stack->Pop<num_t>();
									auto highCutoff    = stack->Pop<num_t>();
									auto lowCutoff     = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									spec.ApplyHighpass(highCutoff, dbPerOctave);
									spec.ApplyLowpass(lowCutoff, dbPerOctave);

									stack->Push(spec);
									trace("0x%04x spectrum.bandpass(%d, %f, %f, %f)", ip-program->bytecode-1, lowCutoff, highCutoff, dbPerOctave);
									break;
								}

							case 0xB33: // spectrum.peak(num harmonic, num width, num db gain)
								{
									auto dbGain   = stack->Pop<num_t>();
									auto width    = stack->Pop<num_t>();
									auto harmonic = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									spec.ApplyPeak(harmonic, width, dbGain);

									stack->Push(spec);
									trace("0x%04x spectrum.peak(%d, %f, %f, %f)", ip-program->bytecode-1, harmonic, width, dbGain);
									break;
								}

							case 0xB40: // spectrum.keepPowX(num harmonic, num factor, num power)
								{
									auto power    = stack->Pop<num_t>();
									auto factor   = stack->Pop<num_t>();
									auto harmonic = stack->Pop<num_t>();

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();

									int32_t prevLog = -2;

									for (uint32_t i=zifloord(harmonic); i<spec.size; i++)
									{
										int32_t curLog = zifloord(zlogd((double)(i-harmonic+2) / factor) / zlogd(power));
										
										if (curLog != prevLog)
										{
											prevLog = curLog;
										}
										else
										{
											spec.data[i] = complex_t(0.0);
										}
									}

									stack->Push(spec);
									trace("0x%04x spectrum.keepPowX(%d, %f, %f, %f)", ip-program->bytecode-1, harmonic, factor, power);
									break;
								}
									/*
kKeepPowX               = 0xB40
kKeepEvery              = 0xB41
kRemoveEvery            = 0xB42
kRemoveRandomAbove      = 0xB43
kRemoveRandomBelow      = 0xB44
kRemovePowX             = 0xB45
*/
							case kOpVoicePitch: // num
								{
									stack->Push(voice->pitch);
									trace("0x%04x push voice pitch", ip-program->bytecode-1, voice->pitch);
									break;
								}

							case kOpStereoWidth: // sample -> sample
								{
									num_t width = stack->Pop<num_t>();
									ZBlockBufferInternal& block = stack->Pop<ZBlockBufferInternal>();

									for (uint32_t i=0; i<block.numSamples; i++)
									{
										num_t mean = (block.samples[i].d[0] + block.samples[i].d[1])/2.0;
										num_t diff = (block.samples[i].d[1] - block.samples[i].d[0]) * width * 0.5;

										block.samples[i] = sample_t(mean - diff, mean + diff);
									}

									stack->Push(block);
									trace("0x%04x stereowidth(%f)", ip-program->bytecode-1, width);
									break;
								}

							case kOpPan: // sample -> sample
								{
									num_t width = stack->Pop<num_t>();
									ZBlockBufferInternal& block = stack->Pop<ZBlockBufferInternal>();

									for (uint32_t i=0; i<block.numSamples; i++)
									{
										num_t mean = (block.samples[i].d[0] + block.samples[i].d[1])/2.0;
										num_t diff = (block.samples[i].d[1] - block.samples[i].d[0]) * width * 0.5;

										block.samples[i] = sample_t(mean - diff, mean + diff);
									}

									stack->Push(block);
									trace("0x%04x stereowidth(%f)", ip-program->bytecode-1, width);
									break;
								}
						}
					}

/*
				case kOpCallFunc:
					{
						switch (argument)
						{
							case 0:
								numStack[-1] = zsind(numStack[-1]);
								ip+=2;
								break;
							case 1:
								numStack[-1] = zcosd(numStack[-1]);
								ip+=2;
								break;
							case 8:
								numStack[-1] = zexpd(numStack[-1]);
								ip+=2;
								break;
						}

						break;
					}
					*/
			}
		}
	}
}

void ZVirtualMachine::CreateNodeInstances(ZVMProgram* program, uint32_t section)
{
	section;
	//opcode_index_t start = program->sections[section], end = program->sections[section+1];

	// Create array
	nodeInstances = new ZNode*[program->bytecodeSize];

	// Reset pointers
	zzeromem(nodeInstances, sizeof(ZNode*)*program->bytecodeSize);
	/*

	// Create node instances where applicable
	ZVMBytecodeIterator it(*program, start, end); 
	while (it.Next())
	{
		if (it.opcode & kOpcodeMaskIsNode)
			nodeInstances[it.i] = CreateNodeFromOpcode(it.opcode);
	}
	*/
}

} // namespace invader