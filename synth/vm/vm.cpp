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
		opcode_index_t opcodeOffset = opcode_index_t(ip-program->bytecode); // used for trace

		opcode_t opcode = *ip++;

		if (opcode & kOpcodeMaskIsNode)
		{
			trace("0x%04x call node, type %x", opcodeOffset, nodeInstances[opcodeOffset]);

			auto node = nodeInstances[opcodeOffset]; 

			if (!node)
			{
				node = nodeInstances[opcodeOffset] = CreateNodeFromOpcode(ip[-1]);

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
					trace("0x%04x halt", opcodeOffset);
					return;
				case kOpPush | kOpTypeNum:
					trace("0x%04x push constant num id %d, value: %f", opcodeOffset, *ip, program->constants[*ip]);
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
					trace("0x%04x push global num, address: 0x%04x", opcodeOffset, *ip);
					stack->Push(globalStorage->Load<num_t>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeSample:
					trace("0x%04x push global sample, address: 0x%04x", opcodeOffset, *ip);
					stack->Push(globalStorage->Load<ZBlockBufferInternal>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeSpectrum:
					trace("0x%04x push global spectrum, address: 0x%04x", opcodeOffset, *ip);
					stack->Push(globalStorage->Load<ZRealSpectrum*>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeWavetable:
					trace("0x%04x push global wavetable, address: 0x%04x", opcodeOffset, *ip);
					stack->Push(globalStorage->Load<ZWaveformWavetable<>*>(*ip++));
					break;
				case kOpPopGlobal | kOpTypeNum:
					trace("0x%04x pop global num, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Store<num_t>(*ip++, stack->Pop<num_t>());
					break;
				case kOpPopGlobal | kOpTypeSample:
					trace("0x%04x pop global sample, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Store<ZBlockBufferInternal>(*ip++, stack->Pop<ZBlockBufferInternal>());
					break;
				case kOpPopGlobal | kOpTypeSpectrum:
					trace("0x%04x pop global spectrum, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Store<ZRealSpectrum*>(*ip++, stack->Pop<ZRealSpectrum*>());
					break;
				case kOpPopGlobal | kOpTypeWavetable:
					trace("0x%04x pop global wavetable, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Store<ZWaveformWavetable<>*>(*ip++, stack->Pop<ZWaveformWavetable<>*>());
					break;
				case kOpResetGlobal | kOpTypeNum:
					trace("0x%04x reet global num, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Reset<num_t>(*ip++);
					break;
				case kOpResetGlobal | kOpTypeSample:
					trace("0x%04x reet global sample, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Reset<ZBlockBufferInternal>(*ip++);
					break;
				case kOpResetGlobal | kOpTypeSpectrum:
					trace("0x%04x reet global spectrum, address: 0x%04x", opcodeOffset, *ip);
					globalStorage->Reset<ZRealSpectrum*>(*ip++);
					break;
/*				case kOpResetGlobal | kOpTypeWavetable:
					trace("0x%04x reet global wavetable, address: 0x%04x", opcodeOffset, *ip);
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
						trace("0x%04x add num x num, %f + %f = %f", opcodeOffset, op1, op2, op1 + op2);
						break;
					}

				case kOpAdd | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->Pop<ZBlockBufferInternal>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] += op2.samples[i];
						stack->Push(op1);
						trace("0x%04x add sample x sample", opcodeOffset);
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
						trace("0x%04x add sample x num (%f)", opcodeOffset, op2);
						break;
					}

				case kOpSubtract | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 - op2);
						trace("0x%04x subtract num x num, %f + %f = %f", opcodeOffset, op1, op2, op1 + op2);
						break;
					}

				case kOpSubtract | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->Pop<ZBlockBufferInternal>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] -= op2.samples[i];
						stack->Push(op1);
						trace("0x%04x subtract sample x sample", opcodeOffset);
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
						trace("0x%04x subtract sample x num (%f)", opcodeOffset, op2);
						break;
					}

				case kOpMultiply | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 * op2);
						trace("0x%04x mul num x num, %f * %f = %f", opcodeOffset, op1, op2, op1 * op2);
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
						trace("0x%04x mul sample x num (%f)", opcodeOffset, op2);
						break;
					}

				case kOpDivide | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 / op2);
						trace("0x%04x div num x num, %f / %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
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
						trace("0x%04x div sample x num (%f)", opcodeOffset, stack->Top<num_t>());
						break;
					}

				case kOpModulo: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(zfmodd(op1, op2));
						trace("0x%04x mod num x num, %f %% %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 == op2) ? 1. : 0.);
						trace("0x%04x equal num x num, %f == %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpNotEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 != op2) ? 1. : 0.);
						trace("0x%04x not equal num x num, %f != %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLessThan: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 < op2) ? 1. : 0.);
						trace("0x%04x less than num x num, %f < %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLessThanOrEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 <= op2) ? 1. : 0.);
						trace("0x%04x less than or equal num x num, %f <= %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpGreaterThan: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 > op2) ? 1. : 0.);
						trace("0x%04x greater than num x num, %f > %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpGreaterThanOrEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 >= op2) ? 1. : 0.);
						trace("0x%04x greater than or equal num x num, %f >= %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLogicalAnd: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1!=0. && op2!=0.) ? 1. : 0.);
						trace("0x%04x and num x num, %f && %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpLogicalOr: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1!=0. || op2!=0.) ? 1. : 0.);
						trace("0x%04x or num x num, %f || %f = %f", opcodeOffset, op1, op2, stack->Top<num_t>());
						break;
					}

				case kOpNot: // num
					{
						num_t op = stack->Pop<num_t>();
						stack->Push((op==0.) ? 1. : 0.);
						trace("0x%04x not num, !%f = %f", opcodeOffset, op, stack->Top<num_t>());
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
						trace("0x%04x minus num, -%f = %f", opcodeOffset, op, stack->Top<num_t>());
						break;
					}

				case kOpMinus | 1: // num
					{
						ZBlockBufferInternal& op = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op.numSamples; i++)
							op.samples[i] *= sample_t(-1.0); // TODO: Use constant!
						stack->Push(op);
						trace("0x%04x minus sample", opcodeOffset);
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
										out.samples[zitruncd(i)] = sample_t(zsind(time*pitchToFrequency(voice->pitch)*kM_PI2));
									}

									stack->Push(out);
									trace("0x%04x osc()", opcodeOffset);
									break;
								}

							case 2: // map_midi_channel(channel, instrument)
								{
									auto instrument = zitruncd(stack->Pop<num_t>());
									auto channel    = zitruncd(stack->Pop<num_t>());
									synth->midiChannelToInstrumentMap[channel] = synth->GetInstrument(instrument);
									trace("0x%04x map_midi_channel(%f, %f)", opcodeOffset, instrument, channel);
									break;
								}

							case 3: // spectrum()
								{
									auto spec = new ZRealSpectrum;
									for (uint32_t i=0; i<spec->size; i++)
										spec->data[i] = complex_t(0.0);
									stack->Push(spec);
									trace("0x%04x spectrum()", opcodeOffset);
									break;
								}

							case 0xB01: // spectrum.toWavetable
								{
									auto amplitudeVariance = stack->Pop<num_t>();
									auto phaseVariance     = stack->Pop<num_t>();
									auto randomSeed        = zitruncd(stack->Pop<num_t>());

									ZRealSpectrum* spectrum = stack->Pop<ZRealSpectrum*>();
									auto wavetable = new ZWaveformWavetable<>(*spectrum, randomSeed, phaseVariance, amplitudeVariance);
									stack->Push(wavetable);
									trace("0x%04x spectrum.toWavetable()", opcodeOffset);
									break;
								}

							case 0xB03: // spectrum.addSine(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = uint32_t(zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();

									if (harmonic>0 && harmonic<spec->size)
										spec->data[harmonic] = complex_t(gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addSine(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB04: // spectrum.addSaw(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=zmax(harmonic, 1); i<spec->size; i++)
										spec->data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addSaw(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB05: // spectrum.addSquare(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=zmax(harmonic, 1); i<spec->size; i+=2)	
										spec->data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addSaw(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB06: // spectrum.addTriangle(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=zmax(harmonic, 1); i<spec->size; i+=2)	
									{
										double n = double(i+1-harmonic);
										double sign = zfmodd((n+1)/2, 2)*2.0-1.0; // alternating sign for harmonics
										spec->data[i] = complex_t(sign / (n*n) * gain);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.addTriangle(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB07: // spectrum.addThirds(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=zmax(harmonic, 1); i<spec->size; i+=3)	
										spec->data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addThirds(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB08: // spectrum.addWhite(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = int32_t(zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=zmax(harmonic, 1); i<spec->size; i+=1)	
										spec->data[i] = complex_t(gain);

									stack->Push(spec);
									trace("0x%04x spectrum.addWhite(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB10: // spectrum.mulWhiteNoise(num seed)
								{
									auto seed = uint32_t(zitruncd(stack->Pop<num_t>()));

									ZRandom r(seed);

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=1; i<spec->size; i++)	
										spec->data[i] *= r.NextUniformDouble();

									stack->Push(spec);
									trace("0x%04x spectrum.mulWhiteNoise(%d)", opcodeOffset, seed);
									break;
								}

							case 0xB11: // spectrum.mulWhiteNoiseDB(num seed, num dB)
								{
									auto db = stack->Pop<num_t>();
									auto seed = uint32_t(zitruncd(stack->Pop<num_t>()));

									ZRandom r(seed);

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=1; i<spec->size; i++)	
										spec->data[i] *= dbToGain(r.NextUniformDouble()*db);

									stack->Push(spec);
									trace("0x%04x spectrum.mulWhiteNoiseDB(%d, %f)", opcodeOffset, seed, db);
									break;
								}

							case 0xB12: // spectrum.spectrumNoise(num seed, num wavelength, num dB)
								{
									auto db = stack->Pop<num_t>();
									auto wavelength = stack->Pop<num_t>();
									auto seed = uint32_t(zitruncd(stack->Pop<num_t>()));

									ZRandom r(seed);

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (double i=wavelength/2; i<spec->size+wavelength/2; i+=wavelength)	
										spec->ApplyPeak(i, wavelength/2.0, r.NextGaussianDouble()*db);
										// this is a liottle different from quiver
											// quiver -> amplitude passed as gain, not db
												// needs test


									stack->Push(spec);
									trace("0x%04x spectrum.spectrumNoise(%d, %f, %f)", opcodeOffset, seed, wavelength, db);
									break;
								}

							case 0xB20: // spectrum.comb(num phase, num frequency)
								{
									auto freq = stack->Pop<num_t>();
									auto phaseOffset = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=1; i<spec->size; i++)	
									{
										double phase = double(i) * freq / spec->size - phaseOffset;
										phase = zfmodd(phase*kM_PI, kM_PI);

										if (phase < 0.0)
											phase += kM_PI;
										spec->data[i] *= zsind(phase);

									}

									stack->Push(spec);
									trace("0x%04x spectrum.comb(%f, %f)", opcodeOffset, phaseOffset, freq);
									break;
								}

							case 0xB21: // spectrum.inverseComb(num phase, num frequency)
								{
									auto freq = stack->Pop<num_t>();
									auto phaseOffset = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();
									for (int32_t i=1; i<spec->size; i++)	
									{
										double phase = double(i) * freq / spec->size - phaseOffset;
										phase = zfmodd(phase*kM_PI, kM_PI);

										if (phase < 0.0)
											phase += kM_PI;

										spec->data[i] *= 1.0 - zsind(phase);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.inverseComb(%f, %f)", opcodeOffset, phaseOffset, freq);
									break;
								}

							case 0xB30: // spectrum.lowpass(num cutoff, num dbPerOctave)
								{
									auto dbPerOctave = stack->Pop<num_t>();
									auto cutoff      = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();
									spec->ApplyLowpass(cutoff, dbPerOctave);

									stack->Push(spec);
									trace("0x%04x spectrum.lowpass(%f, %f)", opcodeOffset, cutoff, dbPerOctave);
									break;
								}

							case 0xB31: // spectrum.highpass(num cutoff, num dbPerOctave)
								{
									auto dbPerOctave = stack->Pop<num_t>();
									auto cutoff      = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();
									spec->ApplyHighpass(cutoff, dbPerOctave);

									stack->Push(spec);
									trace("0x%04x spectrum.highpass(%f, %f)", opcodeOffset, cutoff, dbPerOctave);
									break;
								}

							case 0xB32: // spectrum.bandpass(num lowCutoff, num highCutoff, num dbPerOctave)
								{
									auto dbPerOctave   = stack->Pop<num_t>();
									auto highCutoff    = stack->Pop<num_t>();
									auto lowCutoff     = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();
									spec->ApplyHighpass(highCutoff, dbPerOctave);
									spec->ApplyLowpass(lowCutoff, dbPerOctave);

									stack->Push(spec);
									trace("0x%04x spectrum.bandpass(%d, %f, %f)", opcodeOffset, lowCutoff, highCutoff, dbPerOctave);
									break;
								}

							case 0xB33: // spectrum.peak(num harmonic, num width, num db gain)
								{
									auto dbGain   = stack->Pop<num_t>();
									auto width    = stack->Pop<num_t>();
									auto harmonic = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();
									spec->ApplyPeak(harmonic, width, dbGain);

									stack->Push(spec);
									trace("0x%04x spectrum.peak(%d, %f, %f)", opcodeOffset, harmonic, width, dbGain);
									break;
								}

							case 0xB40: // spectrum.keepPowX(num harmonic, num factor, num power)
								{
									auto power    = stack->Pop<num_t>();
									auto factor   = stack->Pop<num_t>();
									auto harmonic = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();

									int32_t prevLog = -2;

									for (uint32_t i=zitruncd(harmonic); i<spec->size; i++)
									{
										int32_t curLog = zitruncd(zlogd((double)(i-harmonic+2) / factor) / zlogd(power));
										
										if (curLog != prevLog)
										{
											prevLog = curLog;
										}
										else
										{
											spec->data[i] = complex_t(0.0);
										}
									}

									stack->Push(spec);
									trace("0x%04x spectrum.keepPowX(%d, %f, %f)", opcodeOffset, harmonic, factor, power);
									break;
								}


							case 0xB45: // spectrum.removePowX(num harmonic, num factor, num power)
								{
									auto power    = stack->Pop<num_t>();
									auto factor   = stack->Pop<num_t>();
									auto harmonic = stack->Pop<num_t>();

									auto spec = stack->Pop<ZRealSpectrum*>();

									int32_t prevLog = -2;

									for (uint32_t i=zitruncd(harmonic); i<spec->size; i++)
									{
										int32_t curLog = zitruncd(zlogd((double)(i-harmonic+2) / factor) / zlogd(power));
										
										if (curLog != prevLog)
										{
											prevLog = curLog;
											spec->data[i] = complex_t(0.0);
										}
									}

									stack->Push(spec);
									trace("0x%04x spectrum.removePowX(%d, %f, %f)", opcodeOffset, harmonic, factor, power);
									break;
								}

							case 0xB41: // spectrum.keepEvery(num harmonic, num factor)
								{
									auto n        = zitruncd(stack->Pop<num_t>());
									auto harmonic = zitruncd(stack->Pop<num_t>());

									auto spec = stack->Pop<ZRealSpectrum*>();

									for (uint32_t i=harmonic; i<spec->size; i++)
									{
										double mult = (((i-harmonic) % n)==0)	? 1.0 : 0.0;
										spec->data[i] *= mult;
									}

									stack->Push(spec);
									trace("0x%04x spectrum.keepEvery(%d, %d)", opcodeOffset, harmonic, n);
									break;
								}

							case 0xB42: // spectrum.removeEvery(num harmonic, num factor)
								{
									auto n        = zitruncd(stack->Pop<num_t>());
									auto harmonic = zitruncd(stack->Pop<num_t>());

									auto spec = stack->Pop<ZRealSpectrum*>();

									for (uint32_t i=harmonic; i<spec->size; i++)
									{
										double mult = (((i-harmonic) % n)==0)	? 0.0 : 1.0;
										spec->data[i] *= mult;
									}

									stack->Push(spec);
									trace("0x%04x spectrum.keepEvery(%d, %d)", opcodeOffset, harmonic, n);
									break;
								}

							case 0xB43: // spectrum.removeRandomAbove(num harmonic, num seed, num threshold)
								{
									auto threshold = stack->Pop<num_t>();
									auto seed      = zitruncd(stack->Pop<num_t>());
									auto harmonic  = zitruncd(stack->Pop<num_t>());

									auto spec = stack->Pop<ZRealSpectrum*>();

									ZRandom r(seed);

									for (uint32_t i=zmax(0, harmonic)+1; i<spec->size; i++) // We don't remove the first harmonic
									{
										if (r.NextUniformDouble() < threshold)
											spec->data[i] = complex_t(0.0);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.removeRandomAbove(%d, %d, %f)", opcodeOffset, harmonic, seed, threshold);
									break;
								}

							case 0xB44: // spectrum.removeRandomBelow(num harmonic, num seed, num threshold)
								{
									auto threshold = stack->Pop<num_t>();
									auto seed      = zitruncd(stack->Pop<num_t>());
									auto harmonic  = zitruncd(stack->Pop<num_t>());

									auto spec = stack->Pop<ZRealSpectrum*>();

									ZRandom r(seed);

									for (int32_t i=1; i<harmonic && i<spec->size; i++) // We don't remove the first harmonic
									{
										if (r.NextUniformDouble() < threshold)
											spec->data[i] = complex_t(0.0);
									}

									stack->Push(spec);
									trace("0x%04x spectrum.removeRandomBelow(%d, %d, %f)", opcodeOffset, harmonic, seed, threshold);
									break;
								}

							case 0xB50: // spectrum.addCopy(num harmonic, num dB)
								{
									auto gain      = dbToGain(stack->Pop<num_t>());
									auto harmonic  = zmax(1, zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();

									uint32_t j = 0;

									for (int32_t i=harmonic; i<spec->size; i++)
									{
										spec->data[i] += gain * spec->data[j];
									}

									stack->Push(spec);
									trace("0x%04x spectrum.addCopy(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB51: // spectrum.addPitchedCopy(num harmonic, num dB)
								{
									auto gain      = dbToGain(stack->Pop<num_t>());
									auto harmonic  = zmax(1, zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();

									uint32_t j = 0;

									for (int32_t i=harmonic; i<spec->size; i+=harmonic)
									{
										spec->data[i] += gain * spec->data[j];
									}

									stack->Push(spec);
									trace("0x%04x spectrum.addPitchedCopy(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB52: // spectrum.addLayers(num harmonic, num dB)
								{
									auto gain      = dbToGain(stack->Pop<num_t>());
									auto harmonic  = zmax(1, zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();

									for (uint32_t layer = harmonic; layer<spec->size; layer+=harmonic)
									{
										uint32_t j = 0;

										for (int32_t i=layer; i<spec->size; i++)
										{
											spec->data[i] += gain * spec->data[j];
										}
									}

									stack->Push(spec);
									trace("0x%04x spectrum.addLayers(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB53: // spectrum.addPitchedLayers(num harmonic, num dB)
								{
									auto gain      = dbToGain(stack->Pop<num_t>());
									auto harmonic  = zmax(1, zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();

									for (uint32_t layer = harmonic; layer<spec->size; layer+=harmonic)
									{
										uint32_t j = 0;

										for (int32_t i=layer; i<spec->size; i+=layer)
										{
											spec->data[i] += gain * spec->data[j];
										}
									}

									stack->Push(spec);
									trace("0x%04x spectrum.addPitchedLayers(%d, %f)", opcodeOffset, harmonic, gain);
									break;
								}

							case 0xB60: // spectrum.reverse()
								{
									auto spec = stack->Pop<ZRealSpectrum*>();
									ZRealSpectrum in = *spec;

									for (int32_t i=1; i<spec->size; i++)
									{
										spec->data[i] = in.data[in.size-i];
									}

									stack->Push(spec);
									trace("0x%04x spectrum.reverse()", opcodeOffset);
									break;
								}

							case 0xB61: // spectrum.mirror(num harmonic)
								{
									auto harmonic  = zmax(1, zitruncd(stack->Pop<num_t>()));

									auto spec = stack->Pop<ZRealSpectrum*>();
									ZRealSpectrum in = *spec;

									uint32_t j=1;

									for (int32_t i=harmonic-1; i>0; i--)
									{
										spec->data[j++] = in.data[i];
									}

									j=1;
									for (int32_t i=harmonic; i<spec->size; i++)
									{
										spec->data[i] = in.data[j++];
									}

									stack->Push(spec);
									trace("0x%04x spectrum.mirror(%d)", opcodeOffset, harmonic);
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
									trace("0x%04x stereowidth(%f)", opcodeOffset, width);
									break;
								}

							case kOpPan: // sample -> sample
								{
									num_t panRight = stack->Pop<num_t>();
									num_t panLawdB = stack->Pop<num_t>();

									auto panLaw = dbToGain(panLawdB);
									
									auto pan = sample_t(1.0 - panRight, panRight);

									auto scale = sample_t(2.0 - 4.0 * panLaw);
									auto boost = sample_t(1.0 / panLaw);

									auto panMul = boost*(scale*pan*pan + (sample_t(1.0)-scale)*pan);

									ZBlockBufferInternal& block = stack->Pop<ZBlockBufferInternal>();

									for (uint32_t i=0; i<block.numSamples; i++)
									{
										block.samples[i] *= panMul;
									}

									stack->Push(block);
									trace("0x%04x pan(%f, %f)", opcodeOffset, panLawdB, panRight);
									break;
								}

							case kOpVoicePitch: // num voicepitch()
								{
									stack->Push(voice->pitch);
									trace("0x%04x push voicepitch(): %f", opcodeOffset, voice->pitch);
									break;
								}

							case kOpVoiceTime: // num voicetime()
								{
									stack->Push(voice->timeSinceNoteOn);
									trace("0x%04x push voicetime(): %f", opcodeOffset, voice->timeSinceNoteOn);
									break;
								}

							case kOpVoicePos: // num voicepos()
								{
									stack->Push(voice->timeSinceNoteOn*synth->sync.bps);
									trace("0x%04x push voicepos(): %f", opcodeOffset, voice->timeSinceNoteOn*synth->sync.bps);
									break;
								}

							case kOpVoiceId: // num voiceid()
								{
									stack->Push(voice->timeSinceNoteOn*synth->sync.bps);
									trace("0x%04x push voiceid(): %f", opcodeOffset, voice->);
									break;
								}

							case kOpGlobalTime: // num globaltime()
								{
									stack->Push(synth->sync.time);
									trace("0x%04x push globaltime(): %f", opcodeOffset, synth->sync.time);
									break;
								}

							case kOpGlobalPos: // num globalpos()
								{
									stack->Push(synth->sync.pos);
									trace("0x%04x push globalpos(): %f", opcodeOffset, synth->sync.pos);
									break;
								}

							case kOpBPM: // num bpm()
								{
									stack->Push(synth->sync.bpm);
									trace("0x%04x push bpm(): %f", opcodeOffset, synth->sync.bpm);
									break;
								}

							case kOpBPS: // num bps()
								{
									stack->Push(synth->sync.bps);
									trace("0x%04x push bps(): %f", opcodeOffset, synth->sync.bps);
									break;
								}

							case kOpFreqToPitch: // num freqToPitch(num freq)
								{
									num_t x = stack->Pop<num_t>();
									auto y = frequencyToPitch(x);
									stack->Push(y);

									trace("0x%04x push freqToPitch(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpPitchToFreq: // num pitchToFreq(num pitch)
								{
									num_t x = stack->Pop<num_t>();
									auto y = pitchToFrequency(x);
									stack->Push(y);

									trace("0x%04x push pitchToFreq(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpTrunc: // num trunc(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = double(zitruncd(x));
									stack->Push(y);

									trace("0x%04x push trunc(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpRound: // num round(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = double(zifloord(x + 0.5));
									stack->Push(y);

									trace("0x%04x push round(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpCeil: // num ceil(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = double(ziceild(x));
									stack->Push(y);

									trace("0x%04x push ceil(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpFloor: // num floor(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = double(zifloord(x));
									stack->Push(y);

									trace("0x%04x push floor(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpAbs: // num abs(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zabs(x);
									stack->Push(y);

									trace("0x%04x push zabs(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpCos: // num cos(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zcosd(x);
									stack->Push(y);

									trace("0x%04x push cos(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpSin: // num sin(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zsind(x);
									stack->Push(y);

									trace("0x%04x push sin(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpTan: // num tan(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = ztand(x);
									stack->Push(y);

									trace("0x%04x push tan(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpSqrt: // num sqrt(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zsqrtd(x);
									stack->Push(y);

									trace("0x%04x push sqrt(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpLog: // num log(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zlogd(x);
									stack->Push(y);

									trace("0x%04x push log(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpLog2: // num log2(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zlog2d(x);
									stack->Push(y);

									trace("0x%04x push log2(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpLog10: // num log10(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zlog10d(x);
									stack->Push(y);

									trace("0x%04x push log10(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpExp: // num exp(num x)
								{
									num_t x = stack->Pop<num_t>();
									auto y = zexpd(x);
									stack->Push(y);

									trace("0x%04x push exp(%f): %f", opcodeOffset, x, y);
									break;
								}

							case kOpPow: // num pow(num a, num x)
								{
									num_t x = stack->Pop<num_t>();
									num_t a = stack->Pop<num_t>();
									auto y = zpowd(a, x);
									stack->Push(y);

									trace("0x%04x push pow(%f, %f): %f", opcodeOffset, a, x, y);
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