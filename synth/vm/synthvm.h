#pragma once
#include "libs/vm/node.h"
#include "libs/vm/vm.h"
#include "libs/vm/opcodes.h"
#include "libs/base/datatypes/stack.h"

class ZSynth;
class ZInstrument;
class ZVoice;

#ifndef SWIG
	class ZSynthNode : public ZNode
	{
	public:
		ZSynthNode(nodetype_t type) : ZNode(type) {}

		virtual void NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples);
		virtual void NoteOff(uint32_t deltaSamples);
	};
#endif

class ZSynthVirtualMachine : public ZVirtualMachine
{
public:
	ZSynthVirtualMachine(ZSynth* synth, ZInstrument* instrument, ZVoice* voice, double* stack, ZStack<ZBlockBufferInternal>* blockStack);
	virtual ~ZSynthVirtualMachine(void);

	virtual bool ExecuteOpcode(opcode_t opcode, argument_t argument, ZVMProgram* program);
	virtual ZNode* CreateNodeFromOpcode(nodetype_t type);

	// Stacks
	ZStack<ZBlockBufferInternal>* blockStack;

	// Context
	ZSynth*       synth;
	ZInstrument*  instrument;
	ZVoice*       voice;

	ZVMProgram* program;
	opcode_index_t entryPoint;

	ZSyncInfo sync;

	void AllocateNodeInstances(ZVMProgram* program);
	void SetNodeInstance(uint32_t i, ZNode* node) { nodeInstances[i] = node; }
};

