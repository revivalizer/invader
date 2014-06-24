
	local kOpcodeMaskHasArgument = 32768

	-- Base stuff
	kOpReturn    = 0x00
	kOpPush      = 0x10 + kOpcodeMaskHasArgument
	kOpPop       = 0x11
	kOpDec       = 0x12
	kOpClone     = 0x13
	kOpPushVar   = 0x18 + kOpcodeMaskHasArgument
	kOpPopVar    = 0x19 + kOpcodeMaskHasArgument
	kOpCallFunc  = 0x60 + kOpcodeMaskHasArgument
	kOpJump      = 0x80 + kOpcodeMaskHasArgument
	kOpJumpEqual = 0x81 + kOpcodeMaskHasArgument
	kOpJumpNotEqual = 0x82 + kOpcodeMaskHasArgument

	-- Binary opcodes
	kOpAdd                = 0x20
	kOpSubtract           = 0x21
	kOpMultiply           = 0x22
	kOpDivide             = 0x23
	kOpModulo             = 0x24
	kOpEqual              = 0x25
	kOpNotEqual           = 0x26
	kOpLessThan           = 0x27
	kOpLessThanOrEqual    = 0x28
	kOpGreaterThan        = 0x29
	kOpGreaterThanOrEqual = 0x2A
	kOpLogicalAnd         = 0x2B
	kOpLogicalOr          = 0x2C

	-- Unary opcodes
	kOpNot   = 0x40
	kOpPlus  = 0x41
	kOpMinus = 0x42

	-- Bases
	kOpcodeSynthBase     = 1024
	kOpcodeModeller      = 2048
	kOpcodeModellerFunc  = 2048 + 512

	kOpTranslateX          = kOpcodeModellerFunc + 0
	kOpTranslateY          = kOpcodeModellerFunc + 1
	kOpTranslateZ          = kOpcodeModellerFunc + 2
	kOpRotateX             = kOpcodeModellerFunc + 3
	kOpRotateY             = kOpcodeModellerFunc + 4
	kOpRotateZ             = kOpcodeModellerFunc + 5
	kOpScale               = kOpcodeModellerFunc + 6
	kOpScaleUniform        = kOpcodeModellerFunc + 7
	kOpFlipX               = kOpcodeModellerFunc + 8
	kOpFlipY               = kOpcodeModellerFunc + 9
	kOpFlipZ               = kOpcodeModellerFunc + 10

	kOpPushMatrix          = kOpcodeModellerFunc + 16
	kOpPopMatrix           = kOpcodeModellerFunc + 17


