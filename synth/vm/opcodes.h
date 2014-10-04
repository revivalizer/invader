#pragma once

namespace invader {

enum
{
	// Base stuff
	kOpReturn       = 0x000,

	kOpPush         = 0x100,
	kOpPop          = 0x110,
	kOpDec          = 0x120,
	kOpClone        = 0x130,

	kOpPushGlobal   = 0x180,
	kOpPopGlobal    = 0x190,
	kOpResetGlobal  = 0x1A0,

	kOpCallFunc     = 0x600,

	kOpJump         = 0x800,
	kOpJumpEqual    = 0x810,
	kOpJumpNotEqual = 0x820,

	// Binary opcodes
	kOpAdd                = 0x200,
	kOpSubtract           = 0x210,
	kOpMultiply           = 0x220,
	kOpDivide             = 0x230,
	kOpModulo             = 0x240,
	kOpEqual              = 0x250,
	kOpNotEqual           = 0x260,
	kOpLessThan           = 0x270,
	kOpLessThanOrEqual    = 0x280,
	kOpGreaterThan        = 0x290,
	kOpGreaterThanOrEqual = 0x2A0,
	kOpLogicalAnd         = 0x2B0,
	kOpLogicalOr          = 0x2C0,

	// Unary opcodes
	kOpNot   = 0x400,
	kOpPlus  = 0x410,
	kOpMinus = 0x420,

	// Nodes
	kOpNodeADSR         = 0xA01 | kOpcodeMaskIsNode,
	kOpNodeWavetableOsc = 0xA02 | kOpcodeMaskIsNode,
	kOpNodeFilter1      = 0xA03 | kOpcodeMaskIsNode,

	// Spectrum
	// 0xBxx range

	// Voice funcs
	kOpVoicePitch = 0xC01,

	// Other
	kOpStereoWidth           = 0xC11,
	kOpPan                   = 0xC12,
	kOpNodeCompress          = 0xC13,
	kOpNodeSidechainCompress = 0xC14,
	kOpNodeReverb            = 0xC15,
};

// Type identifier
enum
{
	kOpTypeNum       = 0,
	kOpTypeSample    = 1,
	kOpTypeSpectrum  = 2,
	kOpTypeWavetable = 3,
};

} // namespace invader