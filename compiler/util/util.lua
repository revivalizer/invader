function create_table()
	-- this creates a table where you can use the ":" syntax to insert and remove, e.g. "sometable:insert('test')"
	local t = {}
	setmetatable(t, {__index = table})
	return t
end

function warn(test, message)
	if (test==false) then
		print("WARNING: "..message)
	end
end
