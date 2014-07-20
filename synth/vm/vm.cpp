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

void ZVirtualMachine::Run(opcode_t start_address, ZVMProgram* program)
{
	opcode_t* ip = &(program->bytecode[start_address]);

	while (true)
	{
		opcode_t opcode = *ip++;

		if (opcode & kOpcodeMaskIsNode)
		{
			nodeInstances[ip-program->bytecode-1]->Process(this);
		}
		else
		{
			switch (opcode)
			{
				case kOpReturn:
					return;
				case kOpPush | kOpTypeNum:
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
					stack->Push(globalStorage->Load<num_t>(*ip++));
					break;
				case kOpPushGlobal | kOpTypeSample:
					stack->Push(globalStorage->Load<ZBlockBufferInternal>(*ip++));
					break;
				case kOpPopGlobal | kOpTypeNum:
					globalStorage->Store<num_t>(*ip++, stack->Pop<num_t>());
					break;
				case kOpPopGlobal | kOpTypeSample:
					globalStorage->Store<ZBlockBufferInternal>(*ip++, stack->Pop<ZBlockBufferInternal>());
					break;
				case kOpResetGlobal | kOpTypeNum:
					globalStorage->Reset<num_t>(*ip++);
					break;
				case kOpResetGlobal | kOpTypeSample:
					globalStorage->Reset<ZBlockBufferInternal>(*ip++);
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
						break;
					}

				case kOpAdd | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->Pop<ZBlockBufferInternal>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] += op2.samples[i];
						stack->Push(op1);
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
						break;
					}

				case kOpSubtract | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 - op2);
						break;
					}

				case kOpSubtract | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->Pop<ZBlockBufferInternal>();
						ZBlockBufferInternal& op1 = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] -= op2.samples[i];
						stack->Push(op1);
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
						break;
					}

				case kOpMultiply | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 * op2);
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
						break;
					}

				case kOpDivide | 0: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(op1 / op2);
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
						break;
					}

				case kOpModulo: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push(zfmodd(op1, op2));
						break;
					}

				case kOpEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 == op2) ? 1. : 0.);
						break;
					}

				case kOpNotEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 != op2) ? 1. : 0.);
						break;
					}

				case kOpLessThan: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 < op2) ? 1. : 0.);
						break;
					}

				case kOpLessThanOrEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 <= op2) ? 1. : 0.);
						break;
					}

				case kOpGreaterThan: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 > op2) ? 1. : 0.);
						break;
					}

				case kOpGreaterThanOrEqual: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1 >= op2) ? 1. : 0.);
						break;
					}

				case kOpLogicalAnd: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1!=0. && op2!=0.) ? 1. : 0.);
						break;
					}

				case kOpLogicalOr: // num x num
					{
						num_t op2 = stack->Pop<num_t>();
						num_t op1 = stack->Pop<num_t>();
						stack->Push((op1!=0. || op2!=0.) ? 1. : 0.);
						break;
					}

				case kOpNot: // num
					{
						num_t op = stack->Pop<num_t>();
						stack->Push((op==0.) ? 1. : 0.);
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
						break;
					}

				case kOpMinus | 1: // num
					{
						ZBlockBufferInternal& op = stack->Pop<ZBlockBufferInternal>();
						for (uint32_t i=0; i<op.numSamples; i++)
							op.samples[i] *= sample_t(-1.0); // TODO: Use constant!
						stack->Push(op);
						break;
					}

				case kOpCallFunc:
					{
						switch (*ip++)
						{
							case 1: // Osc
								{
									ZBlockBufferInternal out;

									for (double i=0.0; i<out.numSamples; i++)
									{
										double time = voice->timeSinceNoteOn + i/kSampleRate;
										out.samples[zifloord(i)] = sample_t(zsind(time*pitchToFrequency(voice->pitch)*kM_PI2));
									}

									stack->Push(out);
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