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
		["name"] = "wavetableosc",
		["id"] = kOpNodeWavetableOsc,
		["arguments"] = { make_const_type("wavetable") },
		["return_type"] = make_type("sample"),
		["description"] = "Wavetable oscillator.",
	},
}

