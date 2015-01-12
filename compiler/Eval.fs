module Eval

open ErrorHandling;
open ErrorHandlingExtensions;
open ParserTypes;
open ParserLogic;

let eval ast =
    let rec eval n = 
        match n with
        | Number(_, x)                             -> Success x

        | Expr.BinaryOp(_, BinaryOpCode.LogicalOr, x, y)  -> binaryRail (fun x y -> if x<>0.0 || y<>0.0 then 1.0 else 0.0) (eval x) (eval y)

        | Expr.BinaryOp(_, BinaryOpCode.LogicalAnd, x, y) -> binaryRail (fun x y -> if x<>0.0 && y<>0.0 then 1.0 else 0.0) (eval x) (eval y)

        | Expr.BinaryOp(_, BinaryOpCode.NotEqual, x, y) -> binaryRail (fun x y -> if x<>y then 1.0 else 0.0) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.Equal, x, y)    -> binaryRail (fun x y -> if x=y  then 1.0 else 0.0) (eval x) (eval y)

        | Expr.BinaryOp(_, BinaryOpCode.LessThanEqual,    x, y)    -> binaryRail (fun x y -> if x<=y  then 1.0 else 0.0) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.LessThan,         x, y)    -> binaryRail (fun x y -> if x<y   then 1.0 else 0.0) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.GreaterThanEqual, x, y)    -> binaryRail (fun x y -> if x>=y  then 1.0 else 0.0) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.GreaterThan,      x, y)    -> binaryRail (fun x y -> if x>y   then 1.0 else 0.0) (eval x) (eval y)

        | Expr.BinaryOp(_, BinaryOpCode.Add, x, y) -> binaryRail (fun x y -> x + y) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.Sub, x, y) -> binaryRail (fun x y -> x - y) (eval x) (eval y)

        | Expr.BinaryOp(_, BinaryOpCode.Mul, x, y) -> binaryRail (fun x y -> x * y) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.Div, x, y) -> binaryRail (fun x y -> x / y) (eval x) (eval y)
        | Expr.BinaryOp(_, BinaryOpCode.Mod, x, y) -> binaryRail (fun x y -> x % y) (eval x) (eval y)

        | Expr.UnaryOp(_, UnaryOpCode.Plus, x)  -> map (fun x -> +x) (eval x) 
        | Expr.UnaryOp(_, UnaryOpCode.Minus, x) -> map (fun x -> -x) (eval x) 
        | Expr.UnaryOp(_, UnaryOpCode.Not, x)   -> map (fun x -> if x<>0.0 then 0.0 else 1.0) (eval x) 

        | Expr.BinaryOp(_, BinaryOpCode.Dot, _, _) -> Failure (sprintf "Cannot evaluate binary opcode '.'")

        | Expr.BinaryOp(_, t, _, _)                -> Failure (sprintf "Unknown binary opcode %A" t)
        | Expr.UnaryOp(_, t, _)                    -> Failure (sprintf "Unknown binary opcode %A" t)

        | Expr.FuncCall(_, name, args) -> Failure "Function calls not supported in eval"
        | Expr.IdentifierRef(_, id)    -> Failure "Identifier refs not supported in eval"

    let eval' = bind eval
    eval' ast

let expr_eval = parse expr >> eval
