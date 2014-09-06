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
		delete nodeInstances;
		nodeInstances = nullptr;
	}
}

// TODO: FIGURE OUT WHY THIS IS NECCESARY
#pragma warning(disable: 4127)

#if 1
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
			nodeInstances[ip-program->bytecode-1]->Process(this);
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
					stack->Push(globalStorage->Load<ZWaveformWavetable<>>(*ip++));
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
					globalStorage->Store<ZWaveformWavetable<>>(*ip++, stack->Pop<ZWaveformWavetable<>>());
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
				case kOpResetGlobal | kOpTypeWavetable:
					trace("0x%04x reet global wavetable, address: 0x%04x", ip-program->bytecode-1, *ip);
					globalStorage->Reset<ZWaveformWavetable<>>(*ip++);
					break;
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

							case 0x201: // spectrum.toWavetable
								{
									ZRealSpectrum& spectrum = stack->Pop<ZRealSpectrum>();
									ZWaveformWavetable<> wavetable(spectrum);
									stack->Push(wavetable);
									trace("0x%04x spectrum.toWavetable()", ip-program->bytecode-1);
									break;
								}

							case 0x203: // spectrum.addSaw(num harmonic, num gainDB)
								{
									auto gain = dbToGain(stack->Pop<num_t>());
									auto harmonic = uint32_t(zifloord(stack->Pop<num_t>()));

									ZASSERT(harmonic >= 0 && harmonic<spec.size)

									ZRealSpectrum& spec = stack->Pop<ZRealSpectrum>();
									for (uint32_t i=harmonic; i<spec.size; i++)
										spec.data[i] = complex_t(1.0 / double(i+1-harmonic) * gain);
									stack->Push(spec);
									trace("0x%04x spectrum.addSaw(%d, %f)", ip-program->bytecode-1, harmonic, gain);
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
	opcode_index_t start = program->sections[section], end = program->sections[section+1];

	// Create array
	nodeInstances = new ZNode*[program->bytecodeSize];

	// Reset pointers
	zzeromem(nodeInstances, sizeof(ZNode*)*program->bytecodeSize);

	// Create node instances where applicable
	ZVMBytecodeIterator it(*program, start, end); 
	while (it.Next())
	{
		if (it.opcode & kOpcodeMaskIsNode)
			nodeInstances[it.i] = CreateNodeFromOpcode(it.opcode);
	}
}

} // namespace invader