str = [[
### global
### strings
	| channel: 1
	num test = 1
	spectrum spec = spectrum()
	out osc().ADSR(0.1, 0.1, -6, 1.0)
### master
	out strings
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
