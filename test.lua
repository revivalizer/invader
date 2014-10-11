str = [[
### global
	const num a = 2
/*### strings
	| channel: 2
	const num test = 1 + a
	const spectrum spec = spectrum().addSaw(1, 0)
	const wavetable wt = spec.toWavetable()
	out wavetableosc(wt).filter1(3, voicepitch()+24, 0.2).ADSR(0.1, 0.1, -6, 1.0)
### strings2
	| channel: 1
	const spectrum spec2 = spectrum().addSaw(1, 0)
	const wavetable wt2 = spec2.toWavetable()
	out wavetableosc(wt2).filter1(3, voicepitch()+96, 0.2).ADSR(0.1, 0.1, -6, 1.0)
	*/
### test
	| channel: 1
	const spectrum spec2 = spectrum().addSaw(1, 0).inverseComb(5, 10)
	const wavetable wt2 = spec2.toWavetable()
	out wavetableosc(wt2).ADSR(0.1, 0.1, -6, 1.0)
### master
//	out strings.compress(strings2, -36, 20, 10, 10) + strings2.reverb(0, 0, 30, -6, -48, 0, -45, 2, 0.004, -1, 0)
	out test
]]

package.path = ";compiler/?.lua"..package.path
require("compiler")

--local program = compile(str)9
--print(serialize_table(program))

local curfilename = debug.getinfo(1, "S").source:sub(2)
export_binary_and_header(curfilename, compile(str))

--[[

//	   | channel:   0
//	   | min:       C#4
//	   | max:       C-5
//	   | transpose: C-5

//	out osc()

]]
