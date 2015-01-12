// Learn more about F# at http://fsharp.net
// See the 'F# Tutorial' project for more help.

open FParsec;
open ErrorHandling;
open ErrorHandlingExtensions;
open Xunit;
open ParserTypes;
open ParserLogic;
open Eval;

type IL = PushNumber of float
        | BinaryOp of BinaryOpCode
        | UnaryOp of UnaryOpCode
    
// as per
// http://stackoverflow.com/questions/8423571/f-writing-a-function-that-builds-a-list-of-tuples-recursively-a-syntax-error
// http://msdn.microsoft.com/en-us/library/dd233209.aspx
let emitIL n =
    let rec emitr n = seq {
        match n with
        | Number(_, x) -> yield  IL.PushNumber(x)
        | Expr.BinaryOp(_, optype, x, y) -> yield! emitr(x)
                                            yield! emitr(y)
                                            yield  IL.BinaryOp(optype)
        | Expr.UnaryOp(_, optype, x) -> yield! emitr(x)
                                        yield  IL.UnaryOp(optype)
        | Expr.FuncCall(_, name, args) -> ()
        | Expr.IdentifierRef(_, id) -> ()
    }
    Success (emitr n |> Seq.toList) // this function cannot fail atm

let expr_compile = parse expr >> bind emitIL

let test f str =
    match f str with
    | Success(result)   -> printfn "Success: %A" result
    | Failure(errorMsg) -> printfn "Failure: %s" errorMsg


(*
type BinaryOpType = 
    | Add = 100

type IL = PushNumber of float
        | BinaryOp of BinaryOpType

type ExprType = Number of float
              | BinaryOp of BinaryOpType * ExprType * ExprType

let ws = spaces
let str_ws s = pstring s >>. ws

let number = pfloat |>> Number

let opp = new OperatorPrecedenceParser<_,_,_>()
let expr = opp.ExpressionParser

opp.TermParser <- number <|> between (str_ws "(") (str_ws ")") expr

opp.AddOperator(InfixOperator("+", ws, 1, Associativity.Left, fun x y -> ExprType.BinaryOp(BinaryOpType.Add, x, y)))

let test p str =
    match run p str with
    | Success(result, _, _)   -> printfn "Success: %A" result
    | Failure(errorMsg, _, _) -> printfn "Failure: %s" errorMsg

test pfloat "8.6"
test number "1.2"
test expr "1+2+3+4"

let rec eval n = 
    match n with
    | Number(x) -> x
    | BinaryOp(BinaryOpType.Add, x, y) -> eval(x) + eval(y)

let doEval str =
    match run expr str with
    | Success(result, _, _)   -> eval result
    | Failure(errorMsg, _, _) -> 0.0

// as per
// http://stackoverflow.com/questions/8423571/f-writing-a-function-that-builds-a-list-of-tuples-recursively-a-syntax-error
// http://msdn.microsoft.com/en-us/library/dd233209.aspx
let emit n =
    let rec emitr n = seq {
        match n with
        | Number(x) -> yield  IL.PushNumber(x)
        | ExprType.BinaryOp(optype, x, y) -> yield! emitr(x)
                                             yield! emitr(y)
                                             yield  IL.BinaryOp(optype)
    }
    emitr n |> Seq.toList

let doEmit str =
    match run expr str with
    | Success(result, _, _)   -> emit result
    | Failure(errorMsg, _, _) -> []

type program =
    struct
        val size: uint32
        val bytecode: uint16[]
    end

//let t: program = {1, [1]};

type Failure = ParseFailure of string

type Result = Success
            | Failure

*)

[<EntryPoint>]
let main argv = 
    printfn "%A" argv
    test (parse expr) "sin().axe().cos(3)"
    System.Console.ReadLine() |> ignore
    0 // return an integer exit code

