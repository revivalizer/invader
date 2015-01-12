module ParserTypes

open FParsec;

type BinaryOpCode = 
     | Add              = 100
     | Sub              = 101
     | Mul              = 102
     | Div              = 103
     | Mod              = 104
     | LogicalOr        = 110
     | LogicalAnd       = 111
     | Equal            = 120
     | NotEqual         = 121
     | LessThan         = 130
     | LessThanEqual    = 131
     | GreaterThan      = 132
     | GreaterThanEqual = 133
     | Not              = 140
     | Plus             = 141
     | Minus            = 142
     | Dot              = 150

type UnaryOpCode =
     | Plus             = 200
     | Minus            = 201
     | Not              = 202



type Expr = Number        of Position * float
          | BinaryOp      of Position * BinaryOpCode * Expr * Expr
          | UnaryOp       of Position * UnaryOpCode * Expr
          | FuncCall      of Position * string * Expr list
          | IdentifierRef of Position * string

type Type =
     | Sample
     | Num
     | Spectrum
     | Wavetable

type QualifiedType =
     | Const    of Position * Type
     | Varying  of Position * Type

type Statement = Assign of Position * QualifiedType * string * Expr
               | Return of Position * Expr

type Section = Section of Position * string * Statement list

type Program = Program of Position * Section list
