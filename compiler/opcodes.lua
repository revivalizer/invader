
-- Base stuff
kOpReturn       = 0x000

kOpPush         = 0x100
--kOpPop          = 0x110
--kOpDec          = 0x120
--kOpClone        = 0x130

kOpPushGlobal      = 0x180
kOpPopGlobal       = 0x190
kOpResetGlobal     = 0x1A0

-- these aren't needed, let's just use push/pop var instead
--	kOpPushSection  = 0x1A0 + kOpcodeMaskHasArgument
--	kOpPopSection   = 0x1B0 + kOpcodeMaskHasArgument

kOpCallFunc     = 0x600

kOpJump         = 0x800
kOpJumpEqual    = 0x810
kOpJumpNotEqual = 0x820

kOpIsNum    = 0
kOpIsSample = 1

function op_modifier(type_string)
	if (type_string=="num") then
		return kOpIsNum
	elseif (type_string=="sample") then
		return kOpIsSample
	end
end

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

binary_opcodes =
{
	["+"]  = {["num*num"]       = {"num",    kOpAdd + 0},
	          ["sample*sample"] = {"sample", kOpAdd + 1},
	          ["sample*num"]    = {"sample", kOpAdd + 2}},
	["-"]  = {["num*num"]       = {"num",    kOpSub + 0},
	          ["sample*sample"] = {"sample", kOpSub + 1}},
	["-"]  = {["sample*num"]    = {"sample", kOpSub + 2}}, -- don't subtract sample from number
	["*"]  = {["num*num"]       = {"num",    kOpMul + 0},
	          ["sample*num"]    = {"sample", kOpMul + 1}}, -- multiplying samples would ring, so don't do that
	["/"]  = {["num*num"]       = {"num",    kOpDiv + 0},
	          ["sample*num"]    = {"sample", kOpDiv + 1}},
	["%"]  = {["num*num"]       = {"num",    kOpMod + 0}},
	["=="] = {["num*num"]       = {"num",    kOpEqual + 0}},
	["!="] = {["num*num"]       = {"num",    kOpNotEqual + 0}},
	["<"]  = {["num*num"]       = {"num",    kOpLessThan + 0}},
	["<="] = {["num*num"]       = {"num",    kOpLessThanOrEqual + 0}},
	[">"]  = {["num*num"]       = {"num",    kOpGreaterThan + 0}},
	[">="] = {["num*num"]       = {"num",    kOpGreaterThanOrEqual + 0}},
	["&&"] = {["num*num"]       = {"num",    kOpLogicalAnd + 0}},
	["||"] = {["num*num"]       = {"num",    kOpLogicalOr + 0}},
}

unary_opcodes =
{
	["!"]  = {["num"]       = {"num",    kOpNot + 0}},
	["+"]  = {["num"]       = {"num",    kOpPlus + 0},
	          ["sample"]    = {"sample", kOpPlus + 0}},
	["-"]  = {["num"]       = {"num",    kOpMinus + 0},
	          ["sample"]    = {"sample", kOpMinus + 1}},
}
