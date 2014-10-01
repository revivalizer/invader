function type_is_const(type)
	return type.const ~= nil
end

function types_equal(type1, type2)
	if (type1.name~=type2.name) then
		return false
	end

	if (type1.const and type2.const) then
		return true
	end

	if (type1.const==nil and type2.const==nil) then
		return true
	end

	return false
end

function types_compatible(type1, type2)
	if (type1.name~=type2.name) then
		return false
	end

	if type1.const then
		if type2.const then
			return true
		else
			return false
		end
	end

	return true
end

function make_type(name)
	return {["name"] = name}
end

function make_const_type(name)
	return {["name"] = name, ["const"] = "true"}
end

function relaxed_type_combination(type1, type2)
	if (type1.name~=type2.name) then
		return nil
	end

	if type1.const and type2.const then
		return make_const_type(type1.name)
	else
		return make_type(type1.name)
	end

end

function strict_type_combination(type1, type2)
	if (type1.name~=type2.name) then
		return nil
	end

	if type1.const then
		if type2.const then
			return make_const_type(type1.name)
		else
			return nil
		end
	end

	return make_type(type1.name)
end

function type_to_string(type)
	local str = ""

	if (type.const) then
		str = str.."const "
	end

	str = str..type.name

	return str
end

types = {
	["num"] = {
		["methods"] = {
		},
	},
	["sample"] = {
		["methods"] = {
			{
				["name"] = "ADSR",
				["id"] = kOpNodeADSR,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_type("sample"),
				["description"] = "Basic ADSR envelope.",
			},
			{
				["name"] = "filter1",
				["id"] = kOpNodeFilter1,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_type("sample"),
				["description"] = "Basic 4-pole zero delay filter. Arguments: type, cutoff, resonance.",
			},
			{
				["name"] = "stereowidth",
				["id"] = kOpStereoWidth,
				["arguments"] = {make_type("num")},
				["return_type"] = make_type("sample"),
				["description"] = "Stereo width. Argument: num. 0 -> mono, 1 -> no change, more than 1 -> more stereo.",
			},
			{
				["name"] = "pan",
				["id"] = kOpPan,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_type("sample"),
				["description"] = "Pan. First argument is panning law, second is pan (0 for left, 1 for right).",
			},
		},
	},
	["spectrum"] = {
		["methods"] = {
			{
				["name"] = "addSine",
				["id"] = kOpAddSine,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds sine wave to spectrum. Arguments: (num: harmonic (must be larger than 0), num: gain in dB)",
			},
			{
				["name"] = "addSaw",
				["id"] = kOpAddSaw,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds saw wave to spectrum. Arguments: (num: harmonic (must be larger than 0), num: gain in dB)",
			},
			{
				["name"] = "addSquare",
				["id"] = kOpAddSquare,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds square wave to spectrum. Arguments: (num: harmonic (must be larger than 0), num: gain in dB)",
			},
			{
				["name"] = "addTriangle",
				["id"] = kOpAddTriangle,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds triangle wave to spectrum. Arguments: (num: harmonic (must be larger than 0), num: gain in dB)",
			},
			{
				["name"] = "addThirds",
				["id"] = kOpAddThirds,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds third wave to spectrum. Arguments: (num: harmonic (must be larger than 0), num: gain in dB)",
			},
			{
				["name"] = "addWhite",
				["id"] = kOpAddWhite,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds white noise wave to spectrum. Arguments: (num: harmonic (must be larger than 0), num: gain in dB)",
			},
			{
				["name"] = "mulWhiteNoise",
				["id"] = kMulWhiteNoise,
				["arguments"] = {make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Multiplies spectrum with white noise between 0 and 1. Arguments: (num: random seed)",
			},
			{
				["name"] = "mulWhiteNoiseDB",
				["id"] = kMulWhiteNoiseDB,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Multiplies spectrum with white noise, with a gain given in dB. Arguments: (num: ramdom seed, num: gain in dB)",
			},
			{
				["name"] = "spectrumNoise",
				["id"] = kSpectrumNoise,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Multiplies spectrum with white noise between 0 and 1. Arguments: (num: seed, num: frequency, num: noise gain in dB)",
			},
			{
				["name"] = "comb",
				["id"] = kComb,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Multiplies spectrum with comb. Arguments: (num: phase, num: frequency)",
			},
			{
				["name"] = "inverseComb",
				["id"] = kInverseComb,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Multiplies spectrum with inverse comb. Arguments: (num: phase, num: frequency)",
			},
			{
				["name"] = "lowpass",
				["id"] = kLowpass,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Applies lowpass filter to spectrum. Arguments: (num: cutoff harmonic, num: db per octave)",
			},
			{
				["name"] = "highpass",
				["id"] = kHighpass,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Applies highpass filter to spectrum. Arguments: (num: cutoff harmonic, num: db per octave)",
			},
			{
				["name"] = "bandpass",
				["id"] = kBandpass,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Applies bandpass filter to spectrum. Arguments: (num: low cutoff harmonic, num: high cutoff harmonic, num: db per octave)",
			},
			{
				["name"] = "peak",
				["id"] = kPeak,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Applies peak to spectrum. Arguments: (num: harmonic, num: width in harmonics, num: db gain)",
			},
			{
				["name"] = "keepPowX",
				["id"] = kKeepPowX,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Only keeps powers of a number. Arguments: (num: start harmonic, num: factor, num: power)",
			},
			{
				["name"] = "keepEvery",
				["id"] = kKeepEvery,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Only keeps multiplier that are a multiple of a number. Arguments: (num: start harmonic, num: multiplier)",
			},
			{
				["name"] = "removeEvery",
				["id"] = kRemoveEvery,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Removes harmonics that are a multiple of a number. Arguments: (num: start harmonic, num: multiplier)",
			},
			{
				["name"] = "removeRandomAbove",
				["id"] = kRemoveRandomAbove,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Removes random harmonics above a starting harmonic. Arguments: (num: start harmonic, num: seed, num: threshold)",
			},
			{
				["name"] = "removeRandomBelow",
				["id"] = kRemoveRandomBelow,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Removes random harmonics below a harmonic. Arguments: (num: start harmonic, num: seed, num: threshold)",
			},
			{
				["name"] = "removePowX",
				["id"] = kRemovePowX,
				["arguments"] = {make_type("num"), make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Removes harmonics that are a power of a number. Arguments: (num: start harmonic, num: factor, num: power)",
			},
--[[DID I SWAP LAST TWO ARGUMENTS HERE?]]
			{
				["name"] = "addCopy",
				["id"] = kAddCopy,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds a copy of the spectrum to itself. Arguments: (num: harmonic, num: gain dB)",
			},
			{
				["name"] = "addPitchedCopy",
				["id"] = kAddPitchedCopy,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds a pitched copy of the spectrum to itself. Arguments: (num: harmonic, num: gain dB)",
			},
			{
				["name"] = "addLayers",
				["id"] = kAddLayers,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds multiple copies of the spectrum to itself. Arguments: (num: harmonic, num: gain dB)",
			},
			{
				["name"] = "addPitchedLayers",
				["id"] = kAddPitchedLayers,
				["arguments"] = {make_type("num"), make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds multiple pitched copies of the spectrum to itself. Arguments: (num: harmonic, num: gain dB)",
			},
			{
				["name"] = "reverse",
				["id"] = kReverse,
				["arguments"] = {},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Reverses the spectrum. No arguments.",
			},
			{
				["name"] = "mirror",
				["id"] = kMirror,
				["arguments"] = {make_type("num")},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Mirrors the low end of the spectrum about the specified harmonic. Arguments: (num: harmonic)",
			},
			{
				["name"] = "toWavetable",
				["id"] = kOpMakeWavetable,
				["arguments"] = {},
				["return_type"] = make_const_type("wavetable"),
				["description"] = "Make wavetable from spectrum.",
			},
		},
	},
	["wavetable"] = {
		["methods"] = {
		},
	},
}
