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

local transform_ops = {}
transform_ops.translate_x   = kOpTranslateX
transform_ops.translate_y   = kOpTranslateY
transform_ops.translate_z   = kOpTranslateZ
transform_ops.rotate_x      = kOpRotateX
transform_ops.rotate_y      = kOpRotateY
transform_ops.rotate_z      = kOpRotateZ
transform_ops.scale         = kOpScale
transform_ops.scale_uniform = kOpScaleUniform
transform_ops.flip_x        = kOpFlipX
transform_ops.flip_y        = kOpFlipY
transform_ops.flip_z        = kOpFlipZ

function must_loop(iterator)
	return iterator.iterate_count and tonumber(iterator.iterate_count.value)>1
end

function generate_bytecode(node, program)
	if (node.tag=="rule_group_list") then
		for i,v in pairs(node) do
			generate_bytecode(v, program)
		end

	elseif (node.tag=="rule_list") then
		for i,v in ipairs(node) do
			generate_bytecode(v, program)
		end

	elseif (node.tag=="rule") then
		node.label.address = #program.bytecode
		generate_bytecode(node.statement_list, program)

	elseif (node.tag=="statement_list") then
		for i,v in ipairs(node) do
			generate_bytecode(v, program)
		end

	elseif (node.tag=="iterator") then
		if (must_loop(node)) then
			node.iterator_label = create_label(program)

			generate_bytecode(node.iterate_count, program)
			program.bytecode:insert(kOpPushMatrix)

			node.iterator_label.address = #program.bytecode
		else
			program.bytecode:insert(kOpPushMatrix)
		end

		generate_bytecode(node.transform_list, program)

		-- generate bytecode for chained iterator, or rule
		if (node.iterator) then
			generate_bytecode(node.iterator, program)
		else
			generate_bytecode(node.rule_identifier, program)
		end

		if (must_loop(node)) then
			program.bytecode:insert(kOpDec)
			program.bytecode:insert(kOpClone)
			program.bytecode:insert(kOpJumpNotEqual)
			program.bytecode:insert(create_label_ref(node.iterator_label))
			program.bytecode:insert(kOpPop)
		end

		program.bytecode:insert(kOpPopMatrix)

	elseif (node.tag=="transform_list") then
		for i,v in ipairs(node) do
			if (v.tag=="scale") then
				-- special case for three value scale_uniform
				generate_bytecode(v.value_x, program)
				generate_bytecode(v.value_y, program)
				generate_bytecode(v.value_z, program)
			elseif ((v.tag=="flip_x") or (v.tag=="flip_y") or (v.tag=="flip_z")) then
				-- these transforms take no parameters
			else
				-- other transforms take one parameter
				generate_bytecode(v.value, program)
			end

			program.bytecode:insert(transform_ops[v.tag])
		end

	elseif (node.tag=="literal_int" or node.tag=="literal_float") then
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

function transform_rules(program)
	-- create new table with rules grouped by their name
	-- also add labels to rules

	local rule_groups = create_table()
	rule_groups.tag = "rule_group_list"

	local ordered_rule_groups = create_table() -- for iteration

	local count = 0

	for i,rule in ipairs(program.ast.root) do
		rule.label = create_label(program)

		if (rule_groups[rule.identifier.name]) then
			-- rule group exists
			rule_groups[rule.identifier.name]:insert(rule)
		else
			-- new rule group
			rule_groups[rule.identifier.name] = create_table()
			rule_groups[rule.identifier.name].tag = "rule_list"
			rule_groups[rule.identifier.name]:insert(rule)
			rule_groups[rule.identifier.name].id = count

			count = count + 1

			-- insert into order rule group table
			ordered_rule_groups:insert(rule_groups[rule.identifier.name])
		end
	end

	program.rule_groups         = rule_groups
	program.ordered_rule_groups = ordered_rule_groups
end

function generate_packed_rule_table(program)
	--[[
		table layout
			num rule groups
			rule group count*   ->   0,2,3 for two groups with 2 and 1 rules respectively
			rule *
	]]

	local countStr = string.pack("H", 0)
	local ruleStr  = ""

	for i,group in ipairs(program.ordered_rule_groups) do
		countStr = countStr..string.pack("H", #group)

		for j,rule in ipairs(group) do
			-- label, weight, maxdepth, substitute rule
			ruleStr = ruleStr..string.pack("H", program.distinct_labels:get_id(rule.label.address)-1)

			local w = 1
			local md = 0
			local sub = 0xFFFF

			for k,modifier in ipairs(rule.modifier_list) do
				if (modifier.tag=="modifer_weight") then
					w = tonumber(modifer.weight.value)
				elseif (modifier.tag=="modifier_maxdepth") then
					md = tonumber(modifer.maxdepth.value)
				elseif (modifier.tag=="modifier_maxdepth_substitute") then
					md = tonumber(modifier.maxdepth.value)
					sub = program.rule_groups[modifier.substitute_identifier.name].id
				end
			end

			ruleStr = ruleStr..string.pack("H", w)
			ruleStr = ruleStr..string.pack("H", md)
			ruleStr = ruleStr..string.pack("H", sub)
		end
	end

	local table_size = 12

	local tableHeader = ""
	tableHeader = string.pack("I", #program.ordered_rule_groups)
	tableHeader = string.pack("I", table_size)
	tableHeader = string.pack("I", table_size + #countStr)

	return tableHeader..countStr..ruleStr
end

function check_sections(ast)
	ast.named_sections = {}

	for i,section in ipairs(ast.sections) do
		-- check uniquess of section names
		assert(ast.sections[section.name.value]==nil, "duplicate section '"..section.name.value.."' found")

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
		ast.named_sections[section.name.value] = section
	end

	assert(ast.named_sections.master, "master section not found in program")
end

-- Compile program
function compile(str)
	local program = {}
	program.ast = parse(str)

	check_sections(program.ast)
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
