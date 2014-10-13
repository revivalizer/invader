require("re") -- this is the "re" module of lpeg

-- full grammar with captures

print("generate parser")

p = re.compile[[
	start <- ws? ({:tag: '' -> 'root':} {:sections: section_list:} {:match_length: {}:}) -> {}

	section_list <- ({:tag: '' -> 'section_list':} section*) -> {}
	section      <- '### ' ({:tag: '' -> 'section':} {:name: identifier:} {:attributes: attribute_list:} {:statements: statement_list:} ws?) -> {}

	attribute_list <- ({:tag: '' -> 'attribute_list':} attribute*) -> {}
	attribute      <- '|' ws? ({:tag: '' -> 'attribute':} {:name: identifier:} ':' ws? {:value: numeric:}) -> {}


	-- statements
	statement_list <- ({:tag: '' -> 'statement_list':} statement*) -> {}

	statement <- assign_statement / return_statement

	assign_statement <- ({:tag: '' -> 'assign_statement':} {:type: type:} ws {:identifier: identifier:} equal {:operand: expression:}) -> {}
	return_statement <- ({:tag: '' -> 'return_statement':} 'out' ws? {:operand: expression:}) -> {}

	-- types
	type <- ( {:const: ('const' ws) -> 'true':}? {:name: ('sample' / 'num' / 'spectrum' / 'wavetable'):} ) -> {}
--	sample_type            <- {'sample'}
--	num_type               <- {'num'}
--	spectrum_type          <- {'spectrum'}

--	unary       <- ({:tag: '' -> 'unary_op':}   chain_call                ({:operator: unary_operators:}      {:operand:  unary:})*)       -> {}  -- we don't match numerics because e.g. -1.5 would be parse as unary minus, positive 1.5

	--- expressions, functions
	expression  <- logical_or
	logical_or  <- ({:tag: '' -> 'binary_op':}  {:operand1: logical_and:} ({:operator: logical_or_operator:}  {:operand2: logical_or:})?)  -> {}
	logical_and <- ({:tag: '' -> 'binary_op':}  {:operand1: equality:}    ({:operator: logical_and_operator:} {:operand2: logical_and:})?) -> {}
	equality    <- ({:tag: '' -> 'binary_op':}  {:operand1: relational:}  ({:operator: equality_operators:}   {:operand2: equality:})?)    -> {}
	relational  <- ({:tag: '' -> 'binary_op':}  {:operand1: additive:}    ({:operator: relational_operators:} {:operand2: relational:})?)  -> {}
	additive    <- ({:tag: '' -> 'binary_op':}  {:operand1: multitive:}   ({:operator: additive_operators:}   {:operand2: additive:})?)    -> {}
	multitive   <- ({:tag: '' -> 'binary_op':}  {:operand1: unary:}       ({:operator: multitive_operators:}  {:operand2: multitive:})?)   -> {}
	unary       <- ({:tag: '' -> 'unary_op':}   !numeric                   {:operator: unary_operators:}      {:operand:  unary:})         -> {} / chain_call -- we don't match numerics because e.g. -1.5 would be parse as unary minus, positive 1.5. unary cannot be optional, because then chain couldn't complete!
	chain_call  <- ({:tag: '' -> 'binary_op':}  {:operand1: primary:}     ({:operator: chain_operator:}       {:operand2: chain_call:})?)  -> {}

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
	-- hey... '!' is not the not operator

]]

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

-- promotes nodes if parents are empty (lots of empty matches given the current parsing method for binary and unary ops)
function transform_promote_matches(node)
	if (type(node)=="table") then
		if (node.tag=="binary_op" and node.operator==nil) then
			return transform_promote_matches(node.operand1)
		elseif (node.tag=="unary_op" and node.operator==nil) then
			return transform_promote_matches(node[1])
		else
			-- standard case
			for i,v in pairs(node) do
				node[i] = transform_promote_matches(v)
			end

			return node
		end
	else
		-- not table, no transform
		return node
	end
end

function parse(str)
	local ast = p:match(str)

	-- Check if match could be found
	if (ast) then
		-- Check if we mathed entire input string
 		if (ast.match_length==(str:len()+1)) then
			return transform_associativity(transform_promote_matches(ast))
--			return ast
		else
			error("Parse error! Didn't match all input.")
		end
	else
		print("Parse error!")
	end
end
