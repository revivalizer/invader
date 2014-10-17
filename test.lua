str = [[
### global
	const num a = 2
	const num pi = 3.141592653589793

	const num kProfileGauss = 0
	const num kProfileBox = 1
	const num kProfileInvGauss = 2
	const num kProfilePure = 3

	const num kModeNatural = 0
	const num kModeMetallic = 1

### strings
	| channel: 2
	const num test = 1 + a
	const spectrum spec = spectrum().addSaw(1, 0)
	const wavetable wt = spec.toPadWavetable(    3661, kModeNatural, kProfileGauss,      2.7, 1.5, 1.0,    0.0, 1.0, 0.0   )
	out wavetableosc(voicepitch(), wt).filter1(3, voicepitch()+24, 0.2).ADSR(0.1, 0.1, -6, 1.0)
/*### strings2
	| channel: 1
	const spectrum spec2 = spectrum().addSaw(1, 0)
	const wavetable wt2 = spec2.toWavetable()
	out wavetableosc(voicepitch(), wt2).filter1(3, voicepitch()+96, 0.2).ADSR(0.1, 0.1, -6, 1.0)
	*/
### test
	| channel: 1
//	const spectrum spec2 = spectrum().addSaw(1, 0).addCopy(10, 12)
	const spectrum spec2 = spectrum().addSaw(1, 0)
	const wavetable wt2 = spec2.toWavetable(34, 0.3, 2.0)
//	const wavetable wt3 = spec2.toPadWavetable(    3661, 1, 0,      0.7, 1.0, 1.0,    0.0, 1.0, 0.0   )
//	const wavetable wt3 = spec2.toPadWavetable(    3661, kModeNatural, kProfileGauss,      2.7, 1.5, 1.0,    0.0, 1.0, 0.0   )
//	out wavetableosc(voicepitch(), wt2).filter1(3, voicepitch()+48, 0.2).ADSR(0.1, 0.1, -6, 1.0)
	out wavetableosc(voicepitch()+sin(voicetime()*24)*12, wt2).filter1(3, voicepitch()+48, 0.2).ADSR(0.1, 0.1, -6, 1.0)
### master
//	out strings.compress(strings2, -36, 20, 10, 10) + strings2.reverb(0, 0, 30, -6, -48, 0, -45, 2, 0.004, -1, 0)
//	out test.reverb(0, 0, 30, -6, -48, 0, -45, 2, 0.004, -1, 0)
	out test.gain(0) + strings
]]

package.path = ";compiler/?.lua"..package.path
require("compiler")

--local program = compile(str)9
--print(serialize_table(program))

local curfilename = debug.getinfo(1, "S").source:sub(2)
local t = os.clock()
export_binary_and_header(curfilename, compile(str))
print(os.clock()-t)

--[[

//	   | channel:   0
//	   | min:       C#4
//	   | max:       C-5
//	   | transpose: C-5

//	out osc()

]]
