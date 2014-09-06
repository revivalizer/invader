require 'parser'
require 'export'
require 'opcodes'
require 'types'
require 'functions'

require("util.serialize")
require("util.unique_set")
require("util.util")

-- byte size of types
local kNumSize = 8
local kSampleSize = 2*8*16*2 -- stereo * double size * block size * oversampling
local kSpectrumSize = 8*2*2048
--local kWavetableSize = 128*4 + kSpectrumSize + 2*4
local kWavetableSize = 33292

function create_label(program)
	local label = create_table()
	program.labels:insert(label)
	label.id = #program.labels
	label.address=-1
	return label
end

function create_label_ref(label)
	local ref = create_table()
	ref.tag="label_ref"
	ref.id=label.id
	return ref
end

function update_labels(program)
	-- create sorted label list
	local sorted_labels = {}
	for i,v in ipairs(program.labels) do
		sorted_labels[i] = v
	end
	table.sort(sorted_labels, function(a,b) return a.address < b.address end)

	-- create distinct address set
	program.distinct_labels = create_unique_set()
	for i,label in ipairs(sorted_labels) do
		program.distinct_labels:get_id(label.address)
	end

	-- update label references in bytecode
	for i,opcode in ipairs(program.bytecode) do
		if (type(opcode)=="table") then
			if (opcode.tag=="label_ref") then
				program.bytecode[i] = program.distinct_labels:get_id(program.labels[opcode.id].address)-1 -- (index is 1 offset)
			end
		end
	end

	-- generate new label set from sorted, distinct set
	program.labels = program.distinct_labels:to_table()
end

function create_section(name)
	local section = create_table()

	section.tag = "section"

	section.statements     = create_table()
	section.statements.tag = "statement_list"

	section.name       = {}
	section.name.value = "const_global"
	section.name.tag   = "identifier"

	section.attributes = {}

	return section
end

function check_sections(program)
	-- check global is first and master last
	assert(program.ast.sections[1], "no sections found")
	assert(program.ast.sections[1].name.value=="global", "first section must be 'global'")
	assert(program.ast.sections[#program.ast.sections].name.value=="master", "last section must be 'master'")

	local const_section = create_section("const_global")
	table.insert(program.ast.sections, 1, const_section)

	-- setup whether section must produce output
	for i,section in ipairs(program.ast.sections) do
		if (i==1 or i==2) then -- global and const_global don't produce output
			section.mustOutput = false
		else
			section.mustOutput = true
		end
	end

	program.named_sections = {}

	for i,section in ipairs(program.ast.sections) do
		-- check uniquess of section names
		assert(program.named_sections[section.name.value]==nil, "duplicate section '"..section.name.value.."' found")

		-- check that section has an output
		local didOutput = false

		for j,statement in ipairs(section.statements) do
			if (statement.tag=="return_statement") then
				didOutput = true

				statement.section_name = section.name.value
			end
		end

		if (section.mustOutput) then
			assert(didOutput==true, "no output from section '"..section.name.value.."'")
		else
			assert(didOutput==false, "unexpected output from section '"..section.name.value.."'")
		end

		-- warn if output is not last statement
		-- this a little wrong... multiple outputs in section will still work I guess
		-- TODO: rewrite to check multiple outs
		--       out must be last statement in section
		if (#section.statements > 0) then
			warn(section.statements[#section.statements].tag=="return_statement" or section.mustOutput==false, "statements after return in section '"..section.name.value.."', they will be ignored")
		end

		-- add to named sections
		program.named_sections[section.name.value] = section
	end

	assert(program.named_sections.master, "master section not found in program")
end

function is_instrument(section)
	local name = section.name.value

	if (name=="const_global" or name=="global" or name=="master") then
		return false
	else
		return true
	end
end

function check_attributes(program)
	program.named_attributes = {}

	for i,section in ipairs(program.ast.sections) do
		if (is_instrument(section)) then
			program.named_attributes[section] = {}

			-- at the moment, only the channel attribute is allowed
			assert(#section.attributes==1, "'channel' attribute not set for section '"..section.name.value.."'.")
			assert(section.attributes[1].name.value=="channel", "Unknown attribute '"..section.attributes[1].name.value.."' for section '"..section.name.value.."'.")

			for j,attribute in ipairs(section.attributes) do
				program.named_attributes[section][attribute.name.value] = attribute
			end

			-- check value of channel attr
			local channel_attr = program.named_attributes[section]["channel"]
			assert(channel_attr.value.tag=="literal_int", "'channel' attribute must have integer value in section '"..section.name.value.."'.")
			assert(tonumber(channel_attr.value.value)>=1 and tonumber(channel_attr.value.value)<=16, "'channel' attribute out of range in section '"..section.name.value.."'.")

		else
			assert(#section.attributes==0, "Section '"..section.name.value.."' must not have attribute.")
		end
	end
end

function create_literal_int(value)
	local literal_int = create_table()
	literal_int.value = value
	literal_int.tag = "literal_int"
	return literal_int
end

function create_identifier(name)
	local identifier = create_table()
	identifier.value = name
	identifier.tag   = "identifier"
	return identifier
end

function create_function_call_statement(name, arguments)
	local statement = {
		["tag"] = "function_call",
		["identifier"] = create_identifier(name),
		["arguments"] = arguments,
	}

	return statement
end

function generate_midi_mapping_code(program)
	for i,section in ipairs(program.ast.sections) do
		if (is_instrument(section)) then
			local arguments = {
				create_literal_int(program.named_attributes[section].channel.value.value-1), -- minus 1 because MIDI channels are 0-15 in VST standard, but 1-16 in most GUIs
				create_literal_int(tostring(get_instrument_id(section)))
			}

			local statement = create_function_call_statement("map_midi_channel", arguments)
			table.insert(program.named_sections.const_global.statements, statement)
		end
	end
end


function find_last_assign_statement_in_section(section)
	-- looping backwards. return first assign statement
	for i=#section.statements,1,-1 do
		if (section.statements[i].tag=="assign_statement") then
			return section.statements[i]
		end
	end

	return nil
end

-- generate linked list of parent scopes
-- each assign statement generates a new scope
function generate_scope_lists(program)
	local section_parent_scope = find_last_assign_statement_in_section(program.named_sections.global) or find_last_assign_statement_in_section(program.named_sections.const_global)

	for i,section in ipairs(program.ast.sections) do
		local parent_scope = nil

		if (i==1) then
			-- no parent scope in const global
		elseif (i==2) then
			-- parent scope is const section in global section
			parent_scope = find_last_assign_statement_in_section(program.named_sections.const_global)
		else
			-- for all other sections, the default parent scope is applicable
			parent_scope = section_parent_scope
		end

		for j,statement in ipairs(section.statements) do
			program.parent_scope_refs[statement] = parent_scope

			if (statement.tag=="assign_statement") then
				parent_scope = statement
			end
		end
	end
end

-- check that all variable references are found in scope
function find_variable_in_scope(program, var_name, scope)
	if (scope==nil) then
		return nil
	end

	-- scope is an assign statement
	if (scope.identifier.value==var_name) then
		return scope
	else
		return find_variable_in_scope(program, var_name, program.parent_scope_refs[scope])
	end
end

function check_variable_refs_recursive(program, node, scope)
	if (node.tag=="variable_ref") then
		local res_scope = find_variable_in_scope(program, node.identifier.value, scope)

		assert(res_scope, "Variable '"..node.identifier.value.."' not found")
		program.variable_refs[node] = res_scope
	else
		for i,child in pairs(node) do
			if (type(child)=="table") then
				check_variable_refs_recursive(program, child, scope)
			end
		end
	end
end

function check_variable_refs(program)
	for i,section in ipairs(program.ast.sections) do
		for j,statement in ipairs(section.statements) do
			if (statement.tag=="assign_statement" or statement.tag=="return_statement") then
				check_variable_refs_recursive(program, statement.operand, program.parent_scope_refs[statement])
			end
		end
	end
end

-- this looks at all variable refs in master section, and change them to section_refs if match found
function mark_section_refs_recursive(program, node, scope)
	if (node.tag=="variable_ref") then
		local section = program.named_sections[node.identifier.value]
		if (section) then
			node.tag = "section_ref"
			program.section_refs[node] = section
		end
	else
		for i,child in pairs(node) do
			if (type(child)=="table") then
				mark_section_refs_recursive(program, child, scope)
			end
		end
	end
end

function mark_section_refs(program)
	local section = program.named_sections.master

	for j,statement in ipairs(section.statements) do
		if (statement.tag=="assign_statement" or statement.tag=="return_statement") then
			mark_section_refs_recursive(program, statement.operand, program.parent_scope_refs[statement])
		end
	end
end

function extract_const_assignments(program)
	-- move const assignment statements from sections to const_global section
	for i,section in ipairs(program.ast.sections) do
		if (section ~= program.named_sections.const_global) then
			local filtered_statements = create_table()

			for j,statement in ipairs(section.statements) do
				if (statement.tag=="assign_statement" and statement.type.const) then
					program.named_sections.const_global.statements:insert(statement)
				else
					filtered_statements:insert(statement)
				end
			end

			section.statements = filtered_statements
		end
	end
end

function mark_dot_operator(node)
	if (node.tag=="binary_op" and node.operator.type==".") then
		node.tag = "dot_op"
	end

	for i,child in pairs(node) do
		if (type(child)=="table") then
			mark_dot_operator(child)
		end
	end
end

function arg_str_from_table(arguments)
	local s = ""

	for i,argument in ipairs(arguments) do
		if (s~="") then
			s = s..", "
		end

		s = s..type_to_string(argument.type)
	end

	return s
end

function match_function(program, node, function_list, error_generator)
	for i,arg in ipairs(node.arguments) do
		infer_types_recursive(program, arg)
	end

	local funcMatch = nil
	local nameMatches = create_table()

	-- check that types match function description
	for i,func in ipairs(function_list) do
		if (func.name==node.identifier.value) then
			nameMatches:insert(func)

			if (#node.arguments==#func.arguments) then
				local argMatch = true

				for j,arg in ipairs(func.arguments) do
					print(arg)
					argMatch = argMatch and (arg.name==node.arguments[j].type.name)
				end

				if (argMatch) then
					funcMatch = func
				end
			end
		end
	end

	if (funcMatch==nil) then
		local errorString = error_generator(node)

		if (#nameMatches > 0) then
			errorString = errorString.."\nPossible matches:"

			for i,func in ipairs(nameMatches) do
				arguments_type_name = create_table()
				for i,argument_type in ipairs(func.arguments) do
					arguments_type_name:insert(type_to_string(argument_type))
				end

				errorString = errorString.."\n"..type_to_string(func.return_type).." "..func.name.."("..table.concat(arguments_type_name, ", ")..")"
			end
		end

		error(errorString)
	end

	program.function_refs[node] = funcMatch

	-- determine if const (pure)
	local returns_const = type_is_const(funcMatch.return_type) -- if the function returns a const, it really means that it is *pure*, effects no external state.

	for i,arg in ipairs(node.arguments) do
		returns_const = returns_const and type_is_const(arg.type)
	end

	if (returns_const) then
		return make_const_type(funcMatch.return_type.name)
	else
		return funcMatch.return_type
	end
end

-- Type inference
function infer_types_recursive(program, node)
	local type = nil

	if     (node.tag=="variable_ref") then
		type = program.variable_refs[node].type

	elseif (node.tag=="section_ref") then
		type = make_type("sample")

	elseif (node.tag=="dot_op") then
		-- dot operator
			infer_types_recursive(program, node.operand1)

			-- only allow function calls for now
			assert(node.operand2.tag=="function_call", "Only function calls allowed after '.'")

			node.operand2.tag = "method_call"

			program.dot_sibling_refs[node.operand2] = node.operand1

			type = infer_types_recursive(program, node.operand2)

	elseif (node.tag=="binary_op") then
			infer_types_recursive(program, node.operand1)
			infer_types_recursive(program, node.operand2)

			local intype = node.operand1.type.name.."*"..node.operand2.type.name

			local op = binary_opcodes[node.operator.type][intype]

			assert(op, "Type mismatch for operator '"..node.operator.type.."', doesn't support '"..type_to_string(node.operand1.type).."' x '"..type_to_string(node.operand2.type).."'.")

			type = relaxed_type_combination(node.operand1.type, node.operand2.type)

			node.op = op

	elseif (node.tag=="unary_op") then
			infer_types_recursive(program, node.operand)

			local op = unary_opcodes[node.operator.type][node.operand.type]

			assert(op, "Type mismatch for operator '"..node.operator.type.."', doesn't support '"..node.operand.type.."'.")

			type = node.operand.type

			node.op = op

	elseif (node.tag=="literal_int") then
		type = make_const_type("num")

	elseif (node.tag=="literal_float") then
		type = make_const_type("num")

	elseif (node.tag=="method_call") then
		local sibling = program.dot_sibling_refs[node]
		local funcs = types[sibling.type.name].methods

		type = match_function(program, node, funcs, function(node) return "Couldn't match method "..node.identifier.value.."("..arg_str_from_table(node.arguments)..")." end)

	elseif (node.tag=="function_call") then
		type = match_function(program, node, functions, function(node) return "Couldn't match function "..node.identifier.value.."("..arg_str_from_table(node.arguments)..")." end)

	end

	node.type = type

	return type
end

function infer_types(program)
	for i,section in ipairs(program.ast.sections) do
		for j,statement in ipairs(section.statements) do
			if (statement.tag=="assign_statement") then
				local type = infer_types_recursive(program, statement.operand)

				assert(types_compatible(statement.type, type), "Type mismatch in assign statement, trying to assign '"..type_to_string(type).."' to '"..type_to_string(statement.type).."'.")

--				statement.type = type
			elseif (statement.tag=="return_statement") then
				local type = infer_types_recursive(program, statement.operand)

				assert(type.name=="sample", "Type mismatch in return statement, must return sample..")

				statement.type = type
			elseif (statement.tag=="function_call") then
				local type = infer_types_recursive(program, statement)

				statement.type = type
			end
		end
	end
end

function generate_bytecode(program, node)
	-- Generate bytecode for this node, calling children recursively when appropriate
	if (node.tag=="root") then
		for i,section in ipairs(node.sections) do
			generate_bytecode(program, section)
		end

	elseif (node.tag=="section") then
		node.bytecodeStart = #program.bytecode

		for i,statement in ipairs(node.statements) do
			generate_bytecode(program, statement)
		end

		program.bytecode:insert(kOpReturn)

	elseif (node.tag=="assign_statement") then
		if (node.type.name=="num") then
			node.global_address = allocate_global_storage(program, kNumSize, 8)
		elseif (node.type.name=="sample") then
			node.global_address = allocate_global_storage(program, kSampleSize, 16)
		elseif (node.type.name=="spectrum") then
			node.global_address = allocate_global_storage(program, kSpectrumSize, 16)
		elseif (node.type.name=="wavetable") then
			node.global_address = allocate_global_storage(program, kWavetableSize, 16)
		else
			error("Type not handled in assign_statement: "..type_to_string(node.type))
		end

		generate_bytecode(program, node.operand)

		program.bytecode:insert(kOpPopGlobal + op_modifier(node.type))
		program.bytecode:insert(node.global_address) -- (index is 1 offset)

	elseif (node.tag=="return_statement") then
		assert(node.type.name=="sample") -- isn't this true?

		generate_bytecode(program, node.operand)

	elseif (node.tag=="literal_int" or node.tag=="literal_float") then
		node.constant_index = program.constants:get_id(tonumber(node.value))

		program.bytecode:insert(kOpPush)
		program.bytecode:insert(node.constant_index-1) -- (index is 1 offset)

	elseif (node.tag=="variable_ref") then
		local definition_node = program.variable_refs[node]

		program.bytecode:insert(kOpPushGlobal + op_modifier(definition_node.type))
		program.bytecode:insert(definition_node.global_address)

	elseif (node.tag=="section_ref") then
		local section = program.section_refs[node]

		program.bytecode:insert(kOpPushGlobal + op_modifier(make_type("sample")))
		program.bytecode:insert(section.global_address)

	elseif (node.tag=="dot_op") then
		generate_bytecode(program, node.operand1)
		generate_bytecode(program, node.operand2)

	elseif (node.tag=="binary_op") then
		generate_bytecode(program, node.operand1)
		generate_bytecode(program, node.operand2)
		program.bytecode:insert(node.op[2])

	elseif (node.tag=="unary_op") then
		generate_bytecode(program, node.operand)
		program.bytecode:insert(node.op[2])

	elseif (node.tag=="function_call" or node.tag=="method_call") then
		for i,arg in ipairs(node.arguments) do
			generate_bytecode(program, arg)
		end

		local func = program.function_refs[node]

		if (not OpcodeIsNode(func.id)) then
			-- it's only a function if the function isn't a node
			-- in that case, don't insert kOpCallFunc
			program.bytecode:insert(kOpCallFunc)
		end

		program.bytecode:insert(func.id)

	end
end

function get_instrument_id(section)
	assert(is_instrument(section))

	return section.id - 2 -- subtract const_global and global
end

function generate_section_ids(program)
	local count = 0

	-- sections are in the following order: global_const, global, <instruments>, master
	for i,section in ipairs(program.ast.sections) do
		section.id = count
		count = count + 1
	end
end

function generate_section_variables(program)
	for i,section in ipairs(program.ast.sections) do
		section.global_address = allocate_global_storage(program, kSampleSize, 16)
	end
end

function generate_section_start_table(program)
	local startoffset = create_table()

	for i,section in ipairs(program.ast.sections) do
		startoffset[section.id + 1] = section.bytecodeStart
	end

	startoffset:insert(#program.bytecode) -- end marker

	program.section_starts = startoffset
end

function allocate_global_storage(program, size, align)
	-- find next aligned address
	local address = math.floor((program.global_storage_size + (align-1))/align)*align

	-- increase storage amount
	program.global_storage_size = address + size

	-- return address
	return address
end

-- Compile program
function compile(str)
	local program = {}
	program.ast = parse(str)
	print(serialize_table(program.ast))

	program.section_refs      = {}
	program.variable_refs     = {}
	program.function_refs     = {}
	program.parent_scope_refs = {}
	program.dot_sibling_refs  = {}

	check_sections(program)
	generate_section_ids(program)

	check_attributes(program)
	generate_midi_mapping_code(program)

	generate_scope_lists(program)

	mark_section_refs(program)
	check_variable_refs(program)

	mark_dot_operator(program.ast)

	infer_types(program)

	extract_const_assignments(program)

	program.constants           = create_unique_set()
	program.bytecode            = create_table()
	program.labels              = create_table()
	program.global_storage_size = 0

	generate_section_variables(program)

	print(serialize_table(program.ast))
	generate_bytecode(program, program.ast)

	program.constants = program.constants:to_table()

	generate_section_start_table(program)

	print(serialize_table(program.ast))

	-- print bytecode in hex
	if (true) then
		print(serialize_table(program.bytecode))
		for i,v in ipairs(program.bytecode) do
			if (type(v)=="number") then
				program.bytecode[i] = string.format("0x%04X", v)
			end
		end
		print(serialize_table(program.bytecode))
	end

--[[
	--	create_function_list(program)
--	check_expression(program.ast.root)

	program.constants = create_unique_set()
	program.variables = create_unique_set()
	program.labels    = create_table()
	program.bytecode  = create_table()

	transform_rules(program)
	print(serialize_table(program.rule_groups))

	generate_bytecode(program.rule_groups, program)
	program.bytecode:insert(kOpReturn)

	print(serialize_table(program.bytecode))

	update_labels(program)

	print(serialize_table(program.bytecode))

	program.constants = program.constants:to_table()]]

	return program
end
