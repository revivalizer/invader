require 'parser'
require 'export'
require 'opcodes'
require 'functions'

require("util.serialize")
require("util.unique_set")
require("util.util")

-- byte size of types
local kNumSize = 8
local kSampleSize = 2*8*16*2 -- stereo * double size * block size * oversampling

function create_label(program)
	local label = {}
	program.labels:insert(label)
	label.id = #program.labels
	label.address=-1
	return label
end

function create_label_ref(label)
	local ref = {}
	ref.tag="label_ref"
	ref.id=label.id
	return ref
end

function generate_bytecode(node, program)
	if (node.tag=="literal_int" or node.tag=="literal_float") then
		node.constant_index = program.constants:get_id(tonumber(node.value))

		program.bytecode:insert(kOpPush)
		program.bytecode:insert(node.constant_index-1) -- (index is 1 offset)

	elseif (node.tag=="identifier") then
		local func = functions[node.name]

		-- this check will hopefully be superflouos, when we do semantic checking later
		if (func) then
			program.bytecode:insert(kOpCallFunc)
			program.bytecode:insert(func.id)
		end

	end
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

function check_sections(program)
	program.named_sections = {}

	for i,section in ipairs(program.ast.sections) do
		-- check uniquess of section names
		assert(program.named_sections[section.name.value]==nil, "duplicate section '"..section.name.value.."' found")

		-- check that every section has an output
		local didOutput = false

		for j,statement in ipairs(section.statements) do
			if (statement.tag=="return_statement") then
				didOutput = true

				statement.section_name = section.name.value
			end
		end

		assert(didOutput, "no output from section '"..section.name.value.."'")

		-- warn if output is not last statement
		warn(section.statements[#section.statements].tag=="return_statement", "statements after return in section '"..section.name.value.."', they will be ignored")

		-- add to named sections
		program.named_sections[section.name.value] = section
	end

	assert(program.named_sections.master, "master section not found in program")
end

-- generate linked list of parent scopes
-- each assign statement generates a new scope
function generate_scope_lists(program)
	for i,section in ipairs(program.ast.sections) do
		local parent_scope = nil
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

-- Type inference
function infer_types_recursive(program, node)
	local type = nil

	if     (node.tag=="variable_ref") then
		type = program.variable_refs[node].type

	elseif (node.tag=="section_ref") then
		type = "sample"

	elseif (node.tag=="binary_op") then
			infer_types_recursive(program, node.operand1)
			infer_types_recursive(program, node.operand2)

			local intype = node.operand1.type.."*"..node.operand2.type
			local op = binary_opcodes[node.operator.type][intype]

			assert(op, "Type mismatch for operator '"..node.operator.type.."', doesn't support '"..node.operand1.type.."' x '"..node.operand2.type.."'.")

			type = op[1]

			node.op = op

	elseif (node.tag=="unary_op") then
			infer_types_recursive(program, node.operand)

			local op = unary_opcodes[node.operator.type][node.operand.type]

			assert(op, "Type mismatch for operator '"..node.operator.type.."', doesn't support '"..node.operand.type.."'.")

			type = op[1]

			node.op = op

	elseif (node.tag=="literal_int") then
		type = "num"

	elseif (node.tag=="literal_float") then
		type = "num"

	elseif (node.tag=="function_call") then
		for i,arg in ipairs(node.arguments) do
			infer_types_recursive(program, arg)
		end

		local funcMatch = nil
		local nameMatches = create_table()

		-- check that types match function description
		for i,func in ipairs(functions) do
			if (func.name==node.identifier.value) then
				nameMatches:insert(func)

				if (#node.arguments==#func.arguments) then
					local argMatch = true

					for j,arg in ipairs(func.arguments) do
						argMatch = argMatch and (arg==node.arguments[j])
					end

					if (argMatch) then
						funcMatch = func
					end
				end
			end
		end

		if (funcMatch==nil) then
			local errorString = "Couldn't match function "..node.identifier.value.."("..table.concat(node.arguments, ", ")..")."

			if (#nameMatches > 0) then
				errorString = errorString.."\nPossible matches:"

				for i,func in ipairs(nameMatches) do
					errorString = errorString.."\n"..func.return_type.." "..func.name.."("..table.concat(func.arguments, ", ")..")"
				end
			end

			error(errorString)
		end

		program.function_refs[node] = funcMatch
		type = funcMatch.return_type
	end

	node.type = type

	return type
end

-- TODO
-- section name in return statement
-- generally, positions on errors

function infer_types(program)
	for i,section in ipairs(program.ast.sections) do
		for j,statement in ipairs(section.statements) do
			if (statement.tag=="assign_statement") then
				local type = infer_types_recursive(program, statement.operand)

				assert(type==statement.type, "Type mismatch in assign statement, trying to assign "..type.." to "..statement.type..".")

				statement.type = type
			elseif (statement.tag=="return_statement") then
				local type = infer_types_recursive(program, statement.operand)

				assert(type=="sample", "Type mismatch in return statement, must return sample..")

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
		if (node.type=="num") then
			node.global_address = allocate_global_storage(program, kNumSize, 8)
		elseif (node.type=="sample") then
			node.global_address = allocate_global_storage(program, kSampleSize, 16)
		end

		generate_bytecode(program, node.operand)

		program.bytecode:insert(kOpPopGlobal + op_modifier(node.type))
		program.bytecode:insert(node.global_address) -- (index is 1 offset)

	elseif (node.tag=="return_statement") then
		assert(node.type=="sample") -- isn't this true?

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

		program.bytecode:insert(kOpPushGlobal + op_modifier("sample"))
		program.bytecode:insert(section.global_address)

	elseif (node.tag=="binary_op") then
		generate_bytecode(program, node.operand1)
		generate_bytecode(program, node.operand2)
		program.bytecode:insert(node.op[2])

	elseif (node.tag=="unary_op") then
		generate_bytecode(program, node.operand)
		program.bytecode:insert(node.op[2])

	elseif (node.tag=="function_call") then
		for i,arg in ipairs(node.arguments) do
			generate_bytecode(program, arg)
		end

		program.bytecode:insert(kOpCallFunc)

		local func = program.function_refs[node]
		program.bytecode:insert(func.id)
	end
end

function generate_section_ids(program)
	local count = 0

	for i,section in ipairs(program.ast.sections) do
		if (section.name.value ~= "master") then
			section.id = count
			count = count + 1
		end
	end

	program.named_sections.master.id = count
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

	program.section_refs      = {}
	program.variable_refs     = {}
	program.function_refs     = {}
	program.parent_scope_refs = {}

	check_sections(program)
	generate_scope_lists(program)

	mark_section_refs(program)
	check_variable_refs(program)

	infer_types(program)

	program.constants           = create_unique_set()
	program.bytecode            = create_table()
	program.labels              = create_table()
	program.global_storage_size = 0

	generate_section_ids(program)
	generate_section_variables(program)

	generate_bytecode(program, program.ast)

	program.constants = program.constants:to_table()

	generate_section_start_table(program)

	print(serialize_table(program.ast))

	print(#program.constants)

--[[
	print(serialize_table(program.bytecode))
	for i,v in ipairs(program.bytecode) do
		if (type(v)=="number") then
			program.bytecode[i] = string.format("0x%04X", v)
		end
	end
	print(serialize_table(program.bytecode))
]]


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
