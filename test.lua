str = [[
### strings
	num test = 1
	out osc()
### master
	out strings
]]

package.path = ";compiler/?.lua"..package.path
require("compiler")

--local program = compile(str)
--print(serialize_table(program))

local curfilename = debug.getinfo(1, "S").source:sub(2)
export_binary_and_header(curfilename, compile(str))
