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

	// Other
	kOpStereoWidth           = 0xC11,
	kOpPan                   = 0xC12,
	kOpNodeCompress          = 0xC13 | kOpcodeMaskIsNode,
	kOpNodeSidechainCompress = 0xC14 | kOpcodeMaskIsNode,
	kOpNodeReverb            = 0xC15 | kOpcodeMaskIsNode,

	// Voice functions
	kOpVoicePitch  = 0xC01,
	kOpVoiceTime   = 0xC02,
	kOpVoicePos    = 0xC03,
	kOpVoiceId     = 0xC04,
	kOpGlobalTime  = 0xC82,
	kOpGlobalPos   = 0xC83,

	kOpBPM         = 0xCE1,
	kOpBPS         = 0xCE2,

	// Math functions
	kOpFreqToPitch = 0xD00,
	kOpPitchToFreq = 0xD01,
	kOpTrunc       = 0xD10,
	kOpRound       = 0xD11,
	kOpCeil        = 0xD12,
	kOpFloor       = 0xD13,
	kOpAbs         = 0xD14,
	kOpCos         = 0xD20,
	kOpSin         = 0xD21,
	kOpTan         = 0xD22,
	kOpSqrt        = 0xD30,
	kOpLog         = 0xD40,
	kOpLog2        = 0xD41,
	kOpLog10       = 0xD42,
	kOpExp         = 0xD43,
	kOpPow         = 0xD44,
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