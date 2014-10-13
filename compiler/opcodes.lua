
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

kOpIsNum       = 0
kOpIsSample    = 1
kOpIsSpectrum  = 2
kOpIsWavetable = 3

function op_modifier(type_string)
	if (type_string.name=="num") then
		return kOpIsNum
	elseif (type_string.name=="sample") then
		return kOpIsSample
	elseif (type_string.name=="spectrum") then
		return kOpIsSpectrum
	elseif (type_string.name=="wavetable") then
		return kOpIsWavetable
	else
		error(type_to_string(type_string).." type not handled in op_modifier")
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
	          ["sample*sample"] = {"sample", kOpSub + 1},
			  ["sample*num"]    = {"sample", kOpSub + 2}}, -- don't subtract sample from number
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

kOpcodeMaskIsNode = 2^15

function OpcodeIsNode(op)
		if (op >= kOpcodeMaskIsNode) then
			return true
		else
			return false
		end
end

-- Node opcodes
kOpNodeADSR         = 0xA01 + kOpcodeMaskIsNode
kOpNodeWavetableOsc = 0xA02 + kOpcodeMaskIsNode
kOpNodeFilter1      = 0xA03 + kOpcodeMaskIsNode

-- Spectrum opcodes
kOpMakeWavetable        = 0xB01
kOpMakePadWavetable     = 0xB02

kOpAddSine              = 0xB08
kOpAddSaw               = 0xB09
kOpAddSquare            = 0xB0A
kOpAddTriangle          = 0xB0B
kOpAddThirds            = 0xB0C
kOpAddWhite             = 0xB0D

kMulWhiteNoise          = 0xB10
kMulWhiteNoiseDB        = 0xB11
kSpectrumNoise          = 0xB12

kComb                   = 0xB20
kInverseComb            = 0xB21

kLowpass                = 0xB30
kHighpass               = 0xB31
kBandpass               = 0xB32
kPeak                   = 0xB33

kKeepPowX               = 0xB40
kKeepEvery              = 0xB41
kRemoveEvery            = 0xB42
kRemoveRandomAbove      = 0xB43
kRemoveRandomBelow      = 0xB44
kRemovePowX             = 0xB45

kAddCopy                = 0xB50
kAddPitchedCopy         = 0xB51
kAddLayers              = 0xB52
kAddPitchedLayers       = 0xB53

kReverse                = 0xB60
kMirror                 = 0xB61

-- Voice functions
kOpVoicePitch  = 0xC01
kOpVoiceTime   = 0xC02
kOpVoicePos    = 0xC03
kOpVoiceId     = 0xC04
kOpGlobalTime  = 0xC82
kOpGlobalPos   = 0xC83

kOpBPM         = 0xCE1
kOpBPS         = 0xCE2

-- Other
kOpStereoWidth           = 0xC11
kOpPan                   = 0xC12
kOpNodeCompress          = 0xC13 + kOpcodeMaskIsNode
kOpNodeSidechainCompress = 0xC14 + kOpcodeMaskIsNode
kOpNodeReverb            = 0xC15 + kOpcodeMaskIsNode


-- Math functions
kOpFreqToPitch = 0xD00
kOpPitchToFreq = 0xD01
kOpTrunc       = 0xD10
kOpRound       = 0xD11
kOpCeil        = 0xD12
kOpFloor       = 0xD13
kOpAbs         = 0xD14
kOpCos         = 0xD20
kOpSin         = 0xD21
kOpTan         = 0xD22
kOpSqrt        = 0xD30
kOpLog         = 0xD40
kOpLog2        = 0xD41
kOpLog10       = 0xD42
kOpExp         = 0xD43
kOpPow         = 0xD44


