require 'parser'
require 'export'
require 'opcodes'
require 'functions'

require("util.serialize")
require("util.unique_set")
require("util.util")

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
			statement.parent_scope = parent_scope

			if (statement.tag=="assign_statement") then
				parent_scope = statement
			end
		end
	end
end

-- check that all variable references are found in scope
function find_variable_in_scope(var_name, scope)
	if (scope==nil) then
		return nil
	end

	-- scope is an assign statement
	if (scope.identifier.value==var_name) then
		return scope
	else
		return find_variable_in_scope(var_name, scope.parent_scope)
	end
end

function check_variable_refs_recursive(program, node, scope)
	if (node.tag=="variable_ref") then
		local res_scope = find_variable_in_scope(node.identifier.value, scope)

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
				check_variable_refs_recursive(program, statement.operand, statement.parent_scope)
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
			mark_section_refs_recursive(program, statement.operand, statement.parent_scope)
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
			local op = BinaryOpcodes[node.operator.type][intype]

			assert(op, "Type mismatch for operator '"..node.operator.type.."', doesn't support '"..node.operand1.type.."' x '"..node.operand2.type.."'.")

			type = op[1]

			node.op = op
	elseif (node.tag=="unary_op") then
			infer_types_recursive(program, node.operand)

			local op = UnaryOpcodes[node.operator.type][node.operand.type]

			assert(op, "Type mismatch for operator '"..node.operator.type.."', doesn't support '"..node.operand.type.."'.")

			type = op[1]

			node.op = op
	elseif (node.tag=="literal_int") then
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

		node.func_ref = funcMatch
		type = funcMatch.return_type
	end

	node.type = type

	return type
end

-- TODO
-- maybe do function overloading?
-- check binary operands
-- unarys
-- section name in return statement
-- generally, positions on errors
-- parent scope refs should really be in external func, as should func_refs

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
			end
		end
	end
end

-- Compile program
function compile(str)
	local program = {}
	program.ast = parse(str)

	print(serialize_table(program.ast))

	check_sections(program)
	generate_scope_lists(program)

	program.section_refs = {}
	program.variable_refs = {}

	mark_section_refs(program)
	check_variable_refs(program)

	infer_types(program)
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
