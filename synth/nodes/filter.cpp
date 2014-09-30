#include "pch.h"
#include "filter.h"

namespace invader {

ZFilter::ZFilter(nodetype_t type) : ZNode(type)
{
	filter.Reset();
}

void ZFilter::Process(ZVirtualMachine* vm)
{
	// Get arguments
	auto resonance = vm->stack->Pop<num_t>();
	auto cutoff = vm->stack->Pop<num_t>();
	auto type = zifloord(vm->stack->Pop<num_t>());

	// Clamp values
	cutoff    = zclamp(cutoff, -36.0, 132.0); // 132 -> 16744Hz, -36 -> 1.02Hz
	resonance = zclamp(resonance, 0.0, 1.0);

	filter.SetParameters(type, cutoff, resonance);

	// Process
	ZBlockBufferInternal& block = vm->stack->Pop<ZBlockBufferInternal>();
	filter.Process(block);
	vm->stack->Push(block);
}

void ZFilter::NoteOn(double pitch, uint32_t note, uint32_t velocity, uint32_t deltaSamples)
{
	pitch; note; velocity; deltaSamples;
	filter.Reset();
}

void ZFilter::NoteOff(uint32_t deltaSamples)
{
	deltaSamples;
}

} // namespace invader