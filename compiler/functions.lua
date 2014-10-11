local funcoffset = 655 -- random

functions =
{
	{
		["name"] = "osc",
		["id"] = 1,
		["arguments"] = {},
		["return_type"] = make_type("sample"),
		["description"] = "Simple oscillator.",
	},
	{
		["name"] = "map_midi_channel",
		["id"] = 2,
		["arguments"] = {make_type("num"), make_type("num")},
		["return_type"] = make_type("void"),
		["description"] = "Map channel to instrument.",
	},
	{
		["name"] = "spectrum",
		["id"] = 3,
		["arguments"] = {},
		["return_type"] = make_const_type("spectrum"),
		["description"] = "Create spectrum.",
	},
	{
		["name"] = "voicepitch",
		["id"] = kOpVoicePitch,
		["arguments"] = {},
		["return_type"] = make_type("num"),
		["description"] = "Returns pitch of the active voice in semitones.",
	},
	{
		["name"] = "voicepitch",
		["id"] = kOpVoicePitch,
		["arguments"] = {},
		["return_type"] = make_type("num"),
		["description"] = "",
	},
	{
		["name"] = "wavetableosc",
		["id"] = kOpNodeWavetableOsc,
		["arguments"] = { make_const_type("wavetable") },
		["return_type"] = make_type("sample"),
		["description"] = "Wavetable oscillator.",
	},
}

kOpVoicePitch  = 0xC01
kOpVoiceTime   = 0xC02
kOpVoicePos    = 0xC03
kOpVoiceId     = 0xC04
kOpGlobalTime  = 0xC82
kOpGlobalPos   = 0xC83

kOpBPM         = 0xCE1
kOpBPS         = 0xCE2

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



