#pragma once

namespace invader {

enum
{
	// Base stuff
	kOpReturn    = 0x000,

	kOpPush      = 0x100 + kOpcodeMaskHasArgument,
	kOpPop       = 0x110,
	kOpDec       = 0x120,
	kOpClone     = 0x130,

	kOpPushVar   = 0x180 + kOpcodeMaskHasArgument,
	kOpPopVar    = 0x190 + kOpcodeMaskHasArgument,

	kOpCallFunc  = 0x600 + kOpcodeMaskHasArgument,

	kOpJump         = 0x800 + kOpcodeMaskHasArgument,
	kOpJumpEqual    = 0x810 + kOpcodeMaskHasArgument,
	kOpJumpNotEqual = 0x820 + kOpcodeMaskHasArgument,

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

	// Bases
	kOpcodeSynthBase       = 1024,
};

// Type identifier
enum
{
	kOpTypeNum    = 0,
	kOpTypeSample = 1,
};

} // namespace invader