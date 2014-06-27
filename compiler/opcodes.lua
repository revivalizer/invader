
	local kOpcodeMaskHasArgument = 32768

	-- Base stuff
	kOpReturn       = 0x000

	kOpPush         = 0x100 + kOpcodeMaskHasArgument
	kOpPop          = 0x110
	kOpDec          = 0x120
	kOpClone        = 0x130

	kOpPushVar      = 0x180 + kOpcodeMaskHasArgument
	kOpPopVar       = 0x190 + kOpcodeMaskHasArgument

	kOpCallFunc     = 0x600 + kOpcodeMaskHasArgument

	kOpJump         = 0x800 + kOpcodeMaskHasArgument
	kOpJumpEqual    = 0x810 + kOpcodeMaskHasArgument
	kOpJumpNotEqual = 0x820 + kOpcodeMaskHasArgument

	-- Binary opcodes
	kOpAdd                = 0x200
	kOpSub                = 0x210
	kOpMul                = 0x220
	kOpDiv                = 0x230
	kOpMod                = 0x240
	kOpEqual              = 0x250
	kOpNotEqual           = 0x260
	kOpLessThan           = 0x270
	kOpLessThanOrEqual    = 0x280
	kOpGreaterThan        = 0x290
	kOpGreaterThanOrEqual = 0x2A0
	kOpLogicalAnd         = 0x2B0
	kOpLogicalOr          = 0x2C0

	-- Unary opcodes
	kOpNot   = 0x400
	kOpPlus  = 0x410
	kOpMinus = 0x420


BinaryOpcodes =
{
	["+"] = {["num*num"]       = {"num",    kOpAdd + 0}},
	["+"] = {["sample*sample"] = {"sample", kOpAdd + 1}},
	["+"] = {["sample*num"]    = {"sample", kOpAdd + 2}},
	["-"] = {["num*num"]       = {"num",    kOpSub + 0}},
	["-"] = {["sample*sample"] = {"sample", kOpSub + 1}},
	["-"] = {["sample*num"]    = {"sample", kOpSub + 2}}, -- don't subtract sample from number
	["*"] = {["num*num"]       = {"num",    kOpMul + 0}},
	["*"] = {["num*sample"]    = {"sample", kOpMul + 1}},
	["*"] = {["sample*num"]    = {"sample", kOpMul + 2}}, -- multiplying samples would ring, so don't do that
	["/"] = {["num*num"]       = {"num",    kOpDiv + 0}},
	["/"] = {["sample*num"]    = {"sample", kOpDiv + 1}},
}
