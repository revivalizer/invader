module ParserLogic

open FParsec;
open ErrorHandling;
open ErrorHandlingExtensions;
open ParserTypes;
open System.Collections.Generic

type PositionMap() =
    inherit Dictionary<Expr, Position>(HashIdentity.Reference)

let ws = spaces
let str_ws s = pstring s .>> ws

let number = getPosition .>>. pfloat .>> ws |>> Number

let ident =
    let isAsciiIdStart c =
        isAsciiLetter c || c = '_'

    let isAsciiIdContinue c =
        isAsciiLetter c || isDigit c || c = '_'

    identifier (IdentifierOptions(isAsciiIdStart    = isAsciiIdStart,
                                  isAsciiIdContinue = isAsciiIdContinue))

let identref = getPosition .>>. ident .>> ws |>> IdentifierRef

let opp = new OperatorPrecedenceParser<_,_,_>()
let expr = opp.ExpressionParser

let arg_list = sepBy expr (str_ws ",")
let args = between (str_ws "(") (str_ws ")") arg_list
let funccall = pipe3 getPosition ident args (fun pos fname fargs -> FuncCall(pos, fname, fargs))

// For capturing positions of operators
let adjustPosition offset (pos: Position) =
    Position(pos.StreamName, pos.Index + int64 offset,
             pos.Line, pos.Column + int64 offset)

// To simplify infix operator definitions, we define a helper function.
let addInfixOperator str prec assoc mapping =
    let op = InfixOperator(str, getPosition .>> ws, prec, assoc, (),
                           fun opPos leftTerm rightTerm ->
                               mapping
                                   (adjustPosition -str.Length opPos)
                                   leftTerm rightTerm)
    opp.AddOperator(op)

let addPrefixOperator str prec assoc mapping =
    let op = PrefixOperator(str, getPosition .>> ws, prec, assoc, (),
                           fun opPos term ->
                               mapping
                                   (adjustPosition -str.Length opPos)
                                   term)
    opp.AddOperator(op)

opp.TermParser <- number <|> attempt funccall <|> identref <|> between (str_ws "(") (str_ws ")") expr

addInfixOperator  "||" 1 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.LogicalOr, x, y))

addInfixOperator  "&&" 2 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.LogicalAnd, x, y))

addInfixOperator  "!=" 3 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.NotEqual, x, y))
addInfixOperator  "==" 3 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Equal, x, y))

addInfixOperator  "<=" 4 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.LessThanEqual, x, y))
addInfixOperator  "<"  4 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.LessThan, x, y))
addInfixOperator  ">=" 4 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.GreaterThanEqual, x, y))
addInfixOperator  ">"  4 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.GreaterThan, x, y))
 
addInfixOperator  "+"  5 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Add, x, y))
addInfixOperator  "-"  5 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Sub, x, y))

addInfixOperator  "*"  6 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Mul, x, y))
addInfixOperator  "/"  6 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Div, x, y))
addInfixOperator  "%"  6 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Mod, x, y))

addPrefixOperator "+"  7 true                 (fun pos x   -> Expr.UnaryOp(pos, UnaryOpCode.Plus, x))
addPrefixOperator "-"  7 true                 (fun pos x   -> Expr.UnaryOp(pos, UnaryOpCode.Minus, x))
addPrefixOperator "!"  7 true                 (fun pos x   -> Expr.UnaryOp(pos, UnaryOpCode.Not, x))

addInfixOperator  "."  8 Associativity.Left   (fun pos x y -> Expr.BinaryOp(pos, BinaryOpCode.Dot, x, y))

let sample_t    = str_ws "sample"    |>> (fun str -> Sample)
let num_t       = str_ws "num"       |>> (fun str -> Num)
let spectrum_t  = str_ws "spectrum"  |>> (fun str -> Spectrum)
let wavetable_t = str_ws "wavetable" |>> (fun str -> Wavetable)

let type' = sample_t <|> num_t <|> spectrum_t <|> wavetable_t

let const_qt   = pipe2 (getPosition .>> (str_ws "const")) type' (fun pos type' -> Const(pos, type'))
let varying_qt = pipe2 getPosition type' (fun pos type' -> Const(pos, type'))

let qualified_type = const_qt <|> varying_qt

let assign = pipe4 getPosition qualified_type (ident .>> ws .>> str_ws "=") expr (fun pos type' id expr -> Assign(pos, type', id, expr))
let ret = getPosition .>> str_ws "out" .>>. expr |>> Return

let statement = ret <|> assign
let statement_list = many statement
let section = pipe3 getPosition (pstring "### " >>. ident .>> spaces) statement_list (fun pos id statements -> Section(pos, id, statements))

let program = getPosition .>> spaces .>>. many section .>> eof |>> Program

let parse p str =
    match run p str with
    | ParserResult.Success(result, _, _)   -> Success(result)
    | ParserResult.Failure(errorMsg, _, _) -> Failure(errorMsg)

// This line is necceasry to compile without value restriction errors!
let expr_parse = parse expr
