require("re") -- this is the "re" module of lpeg

-- full grammar with captures

p = re.compile[[
	start <- ws? ({:tag: '' -> 'root':} {:sections: section_list:} {:match_length: {}:}) -> {}

	section_list <- ({:tag: '' -> 'section_list':} section*) -> {}
	section      <- '### ' ({:tag: '' -> 'section':} {:name: identifier:} {:statements: statement_list:} ws?) -> {}

	-- statements
	statement_list <- ({:tag: '' -> 'statement_list':} statement*) -> {}

	statement <- assign_statement / return_statement

	assign_statement <- ({:tag: '' -> 'assign_statement':} {:type: type:} ws {:identifier: identifier:} equal {:operand: expression:}) -> {}
	return_statement <- ({:tag: '' -> 'return_statement':} 'out' ws? {:operand: expression:}) -> {}

	-- types
	type <- sample_type / num_type
	sample_type <- 'sample'
	num_type    <- 'num'

	--- expressions, functions
	expression  <- logical_or
	logical_or  <- ({:tag: '' -> 'binary_op':}  {:operand1: logical_and:} {:operator: logical_or_operator:}  {:operand2: logical_or:})  -> {}	/ logical_and
	logical_and <- ({:tag: '' -> 'binary_op':}  {:operand1: equality:}    {:operator: logical_and_operator:} {:operand2: logical_and:}) -> {}	/ equality
	equality    <- ({:tag: '' -> 'binary_op':}  {:operand1: relational:}  {:operator: equality_operators:}   {:operand2: equality:})    -> {}	/ relational
	relational  <- ({:tag: '' -> 'binary_op':}  {:operand1: additive:}    {:operator: relational_operators:} {:operand2: relational:})  -> {}	/ additive
	additive    <- ({:tag: '' -> 'binary_op':}  {:operand1: multitive:}   {:operator: additive_operators:}   {:operand2: additive:})    -> {}	/ multitive
	multitive   <- ({:tag: '' -> 'binary_op':}  {:operand1: unary:}       {:operator: multitive_operators:}  {:operand2: multitive:})   -> {}	/ unary
	unary       <- ({:tag: '' -> 'unary_op':}   !numeric                  {:operator: unary_operators:}      {:operand:  unary:})       -> {}	/ chain_call    -- we don't match numerics because e.g. -1.5 would be parse as unary minus, positive 1.5
	chain_call  <- ({:tag: '' -> 'binary_op':}  {:operand1: primary:}     {:operator: chain_operator:}       {:operand2: chain_call:})  -> {}   / primary

	primary     <- numeric / function_call / variable_ref / parensed_expression

	parensed_expression <- ({:tag: '' -> 'parensed_expression':} {:open_parens: open_parens:} {:expression: expression:} {:close_parens: close_parens:}) -> {}

	additive_operators   <- addition_operator / subtraction_operator
	multitive_operators  <- multiplication_operator / division_operator / modulo_operator
	equality_operators   <- equality_operator / inequality_operator
	relational_operators <- less_than_or_equal_operator / greater_than_or_equal_operator / less_than_operator / greater_than_operator
	unary_operators      <- not_operator / plus_operator / minus_operator

	chain_operator                 <- ({:precedence: '' ->  '2':} {:assoc: '' ->  'left':} {:type:  '.':} ws?) -> {}
	not_operator                   <- ({:precedence: '' ->  '3':} {:assoc: '' -> 'right':} {:type:  '!':} ws?) -> {}
	plus_operator                  <- ({:precedence: '' ->  '3':} {:assoc: '' -> 'right':} {:type:  '+':} ws?) -> {}
	minus_operator                 <- ({:precedence: '' ->  '3':} {:assoc: '' -> 'right':} {:type:  '-':} ws?) -> {}
	multiplication_operator        <- ({:precedence: '' ->  '5':} {:assoc: '' ->  'left':} {:type:  '*':} ws?) -> {}
	division_operator              <- ({:precedence: '' ->  '5':} {:assoc: '' ->  'left':} {:type:  '/':} ws?) -> {}
	modulo_operator                <- ({:precedence: '' ->  '5':} {:assoc: '' ->  'left':} {:type:  '%':} ws?) -> {}
	addition_operator              <- ({:precedence: '' ->  '6':} {:assoc: '' ->  'left':} {:type:  '+':} ws?) -> {}
	subtraction_operator           <- ({:precedence: '' ->  '6':} {:assoc: '' ->  'left':} {:type:  '-':} ws?) -> {}
	less_than_or_equal_operator    <- ({:precedence: '' ->  '8':} {:assoc: '' ->  'left':} {:type: '<=':} ws?) -> {}
	greater_than_or_equal_operator <- ({:precedence: '' ->  '8':} {:assoc: '' ->  'left':} {:type: '>=':} ws?) -> {}
	less_than_operator             <- ({:precedence: '' ->  '8':} {:assoc: '' ->  'left':} {:type:  '<':} ws?) -> {}
	greater_than_operator          <- ({:precedence: '' ->  '8':} {:assoc: '' ->  'left':} {:type:  '>':} ws?) -> {}
	equality_operator              <- ({:precedence: '' ->  '9':} {:assoc: '' ->  'left':} {:type: '==':} ws?) -> {}
	inequality_operator            <- ({:precedence: '' ->  '9':} {:assoc: '' ->  'left':} {:type: '!=':} ws?) -> {}
	logical_and_operator           <- ({:precedence: '' -> '13':} {:assoc: '' ->  'left':} {:type: '&&':} ws?) -> {}
	logical_or_operator            <- ({:precedence: '' -> '14':} {:assoc: '' ->  'left':} {:type: '||':} ws?) -> {}

	function_call           <- ({:tag: '' -> 'function_call':} {:identifier: identifier:} open_parens {:arguments: function_call_arguments:} close_parens ) -> {}
	function_call_arguments <- (expression comma_expression*)? -> {}

	comma_expression  <- comma expression

	variable_ref <- ({:tag: '' -> 'variable_ref':} {:identifier: identifier:}) -> {}

	numeric <- float / integer

	integer       <- ({:tag: '' -> 'literal_int':}     {:value: [-+]? [0-9]+:} ws?) -> {}
	float         <- ({:tag: '' -> 'literal_float':}   {:value: [-+]? [0-9]* '.' [0-9]+:} ws?) -> {} /
	                 ({:tag: '' -> 'literal_float':}   {:value: [-+]? [0-9]+ '.' [0-9]*:} ws?) -> {}

	identifier    <- ({:tag: '' -> 'identifier':} {:value: [a-zA-Z] [a-zA-Z0-9_]*:} ws?) -> {}

	open_parens   <- '(' ws?
	close_parens  <- ')' ws?

	open_brace    <- '{' ws?
	close_brace   <- '}' ws?

	semicolon     <- ';' ws?
	comma         <- ',' ws?

	equal         <- '=' ws?

	-- whitespace

	ws           <- (%s / line_comment / multiline_comment)+
	line_comment <- '//' [^%nl]*
	multiline_comment       <- '/*' multiline_comment_close
	multiline_comment_close <- '*/' / . multiline_comment_close
	--	I am little unhappy with above rule, but couldn't get this to work: (!'*/')* '*/'

]]

--[=[p = re.compile([[
	start        <- ws? ({:root: rule_list:} {:match_length: {}:}) -> {}

	rule_list <- ({:tag: '' -> 'rule_list':} rule*) -> {}

	rule <- ({:tag: '' -> 'rule':} ({:entrypoint: rule_entrypoint:})? 'rule' ws {:identifier: identifier:} {:modifier_list: rule_modifier_list:} open_brace {:statement_list: statement_list:} close_brace) -> {}
	rule_entrypoint <- 'entry' ws integer

	transform <- translate_x / translate_y / translate_z / rotate_x / rotate_y / rotate_z / scale / scale_uniform / flip_x / flip_y / flip_z / hue / saturation / brightness
	transform_list <- ({:tag: '' -> 'transform_list':} transform*) -> {}
	transform_block <- open_brace {:transform_list: transform_list:} close_brace


	iterator <- ({:tag: '' -> 'iterator':} ({:iterate_count: integer:} '*' ws?)? transform_block ({:iterator: iterator:} / {:rule_identifier: identifier:})?) -> {}
--	iterator_statement <- ({:tag: '' -> 'iterator_statement':} ({:iterator: iterator:})? {:rule_identifier: identifier:}) -> {}
--	iterator_statement <- ({:tag: '' -> 'iterator_statement':} {:rule_identifier: identifier:}) -> {}
	statement_list <- ({:tag: '' -> 'statement_list':} iterator*) -> {}


	modifier_weight              <- ({:tag: '' -> 'modifier_weight':}              'w'  ws {:weight:   integer:}) -> {}
	modifier_maxdepth            <- ({:tag: '' -> 'modifier_maxdepth':}            'md' ws {:maxdepth: integer:}) -> {}
	modifier_maxdepth_substitute <- ({:tag: '' -> 'modifier_maxdepth_substitute':} 'md' ws {:maxdepth: integer:} ws? '>' ws? {:substitute_identifier: identifier:}) -> {}

	rule_modifier      <- modifier_weight / modifier_maxdepth_substitute / modifier_maxdepth
	rule_modifier_list <- rule_modifier* -> {}


	translate_x   <- ({:tag: '' -> 'translate_x':}   'x'   ws {:value: expression:}) -> {}
	translate_y   <- ({:tag: '' -> 'translate_y':}   'y'   ws {:value: expression:}) -> {}
	translate_z   <- ({:tag: '' -> 'translate_z':}   'z'   ws {:value: expression:}) -> {}
	rotate_x      <- ({:tag: '' -> 'rotate_x':}      'rx'  ws {:value: expression:}) -> {}
	rotate_y      <- ({:tag: '' -> 'rotate_y':}      'ry'  ws {:value: expression:}) -> {}
	rotate_z      <- ({:tag: '' -> 'rotate_z':}      'rz'  ws {:value: expression:}) -> {}
	scale         <- ({:tag: '' -> 'scale':}         's'   ws {:value_x: expression:} {:value_y: expression:} {:value_z: expression:}) -> {}
	scale_uniform <- ({:tag: '' -> 'scale_uniform':} 's'   ws {:value: expression:}) -> {}
	flip_x        <- ({:tag: '' -> 'flip_x':}        'fx'  ws?                     ) -> {}
	flip_y        <- ({:tag: '' -> 'flip_y':}        'fy'  ws?                     ) -> {}
	flip_z        <- ({:tag: '' -> 'flip_z':}        'fz'  ws?                     ) -> {}
	hue           <- ({:tag: '' -> 'hue':}           'h'   ws {:value: expression:}) -> {}
	saturation    <- ({:tag: '' -> 'saturation':}    'sat' ws {:value: expression:}) -> {}
	brightness    <- ({:tag: '' -> 'brightness':}    'b'   ws {:value: expression:}) -> {}

	expression  <- numeric

	identifier <- ({:tag: '' -> 'identifier':} {:name: [a-zA-Z] [a-zA-Z0-9_]*:} ws?) -> {}

	numeric <- float / integer

	integer       <- ({:tag: '' -> 'literal_int':}     {:value: [-+]? [0-9]+:} ws?) -> {}
	float         <- ({:tag: '' -> 'literal_float':}   {:value: [-+]? [0-9]* '.' [0-9]+:} ws?) -> {} /
			         ({:tag: '' -> 'literal_float':}   {:value: [-+]? [0-9]+ '.' [0-9]*:} ws?) -> {}


	open_parens  <- '(' ws?
	close_parens <- ')' ws?

	open_brace   <- '{' ws?
	close_brace  <- '}' ws?

	ws           <- (%s / line_comment / multiline_comment)+
	line_comment <- '//' [^%nl]*
	multiline_comment       <- '/*' multiline_comment_close
	multiline_comment_close <- '*/' / . multiline_comment_close
	--	I am little unhappy with above rule, but couldn't get this to work: (!'*/')* '*/'

]])]=]

-- This functions transforms right associative binary operators to left associative ones, when neccesary
function transform_associativity(node)
	if (type(node)=="table") then
		if (
				node.tag                     == "binary_op" and
				node.operand2.tag            == "binary_op" and
				node.operator.assoc          == "left"      and
				node.operand2.operator.assoc == "left"      and
				node.operator.precedence     ==  node.operand2.operator.precedence
			)
			then
			-- node and node.operand2 must have their associativity changed.

			-- rotate nodes
			local newroot = node.operand2
			node.operand2 = newroot.operand1
			newroot.operand1 = node

			return transform_associativity(newroot)
		else
			-- standard case
			for i,v in pairs(node) do
				node[i] = transform_associativity(v)
			end

			return node
		end
	else
		return node
	end
end

function parse(str)
	local ast = p:match(str)

	-- Check if match could be found
	if (ast) then
		-- Check if we mathed entire input string
 		if (ast.match_length==(str:len()+1)) then
--			return (ast)
			return transform_associativity(ast)
		else
			error("Parse error! Didn't match all input.")
		end
	else
		print("Parse error!")
	end
end
