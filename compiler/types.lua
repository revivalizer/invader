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
				["arguments"] = {"num", "num", "num", "num"},
				["return_type"] = make_type("sample"),
				["description"] = "Basic ADSR envelope.",
			},
		},
	},
	["spectrum"] = {
		["methods"] = {
			{
				["name"] = "addSaw",
				["id"] = kOpAddSaw,
				["arguments"] = {"num", "num"},
				["return_type"] = make_const_type("spectrum"),
				["description"] = "Adds saw to spectrum. Arguments: (num harmonic, num gain)",
			},
			{
				["name"] = "toWavetable",
				["id"] = kOpAddSaw,
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
