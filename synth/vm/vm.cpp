#include "pch.h"

namespace invader {

ZVirtualMachine::ZVirtualMachine(ZVMProgram* program, ZVMStack* stack)
	: nodeInstances(nullptr)
	, stack(stack)
	, program(program)
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
				case kOpPush + kOpTypeNum:
					stack->PushNum(program->constants[*ip++]);
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
				case kOpPushVar | kOpTypeNum:
					stack->PushNum(globalStorage.LoadNum(*ip++));
					break;
				case kOpPushVar | kOpTypeSample:
					stack->PushSampleBlock(globalStorage.LoadSampleBlock(*ip++));
					break;
				case kOpPopVar | kOpTypeNum:
					globalStorage.StoreNum(*ip++, stack->PopNum());
					break;
				case kOpPopVar | kOpTypeSample:
					globalStorage.StoreSampleBlock(*ip++, stack->PopSampleBlock());
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
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum(op1 + op2);
						break;
					}

				case kOpAdd | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->PopSampleBlock();
						ZBlockBufferInternal& op1 = stack->PopSampleBlock();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] += op2.samples[i];
						break;
					}

				case kOpAdd | 2: // sample x num
					{
						num_t op2 = stack->PopNum();
						ZBlockBufferInternal& op1 = stack->PopSampleBlock();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] += s;
						break;
					}

				case kOpSubtract | 0: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum(op1 - op2);
						break;
					}

				case kOpSubtract | 1: // sample x sample
					{
						ZBlockBufferInternal& op2 = stack->PopSampleBlock();
						ZBlockBufferInternal& op1 = stack->PopSampleBlock();
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] -= op2.samples[i];
						break;
					}

				case kOpSubtract | 2: // sample x num
					{
						num_t op2 = stack->PopNum();
						ZBlockBufferInternal& op1 = stack->PopSampleBlock();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] -= s;
						break;
					}

				case kOpMultiply | 0: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum(op1 * op2);
						break;
					}

				case kOpMultiply | 1: // sample x num
					{
						num_t op2 = stack->PopNum();
						ZBlockBufferInternal& op1 = stack->PopSampleBlock();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] *= s;
						break;
					}

				case kOpDivide | 0: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum(op1 / op2);
						break;
					}

				case kOpDivide | 1: // sample x num
					{
						num_t op2 = stack->PopNum();
						ZBlockBufferInternal& op1 = stack->PopSampleBlock();
						sample_t s = sample_t(op2);
						for (uint32_t i=0; i<op1.numSamples; i++)
							op1.samples[i] /= s;
						break;
					}

				case kOpModulo: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum(zfmodd(op1, op2));
						break;
					}

				case kOpEqual: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1 == op2) ? 1. : 0.);
						break;
					}

				case kOpNotEqual: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1 != op2) ? 1. : 0.);
						break;
					}

				case kOpLessThan: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1 < op2) ? 1. : 0.);
						break;
					}

				case kOpLessThanOrEqual: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1 <= op2) ? 1. : 0.);
						break;
					}

				case kOpGreaterThan: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1 > op2) ? 1. : 0.);
						break;
					}

				case kOpGreaterThanOrEqual: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1 >= op2) ? 1. : 0.);
						break;
					}

				case kOpLogicalAnd: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1!=0. && op2!=0.) ? 1. : 0.);
						break;
					}

				case kOpLogicalOr: // num x num
					{
						num_t op2 = stack->PopNum();
						num_t op1 = stack->PopNum();
						stack->PushNum((op1!=0. || op2!=0.) ? 1. : 0.);
						break;
					}

				case kOpNot: // num
					{
						num_t op = stack->PopNum();
						stack->PushNum((op==0.) ? 1. : 0.);
						break;
					}

				case kOpPlus: // num | sample
					{
						// do nothing
						break;
					}

				case kOpMinus | 0: // num
					{
						num_t op = stack->PopNum();
						stack->PushNum(-op);
						break;
					}

				case kOpMinus | 1: // num
					{
						ZBlockBufferInternal& op = stack->PopSampleBlock();
						for (uint32_t i=0; i<op.numSamples; i++)
							op.samples[i] *= sample_t(-1.0); // TODO: Use constant!
						break;
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

void ZVirtualMachine::CreateNodeInstances(ZVMProgram* program)
{
	// Create array
	nodeInstances = new ZNode*[program->bytecodeSize];

	// Reset pointers
	zzeromem(nodeInstances, sizeof(ZNode*)*program->bytecodeSize);

	// Create node instances where applicable
	ZVMBytecodeIterator it(*program);

	while (it.Next())
	{
		if (it.opcode & kOpcodeMaskIsNode)
			nodeInstances[it.i] = CreateNodeFromOpcode(it.opcode);
	}
}

} // namespace invader