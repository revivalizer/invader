str = [[
### strings
	num test = 1
	out osc().ADSR(0.1, 0.1, 0.5, 1.0)
### master
	out strings
]]

package.path = ";compiler/?.lua"..package.path
require("compiler")

--local program = compile(str)
--print(serialize_table(program))

local curfilename = debug.getinfo(1, "S").source:sub(2)
export_binary_and_header(curfilename, compile(str))
