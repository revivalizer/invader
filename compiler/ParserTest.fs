module ParserTest

open FParsec;
open Xunit;
open ErrorHandling;
open ErrorHandlingExtensions;
open ParserTypes;
open ParserLogic;
open Eval;

let AssertExprEvalEqual str value =
    Assert.Equal((parse (expr .>> eof) >> eval) str, Success(value))
    
let AssertParseSuccess p str =
    Assert.True(success (parse (p .>> eof) str))

let AssertParseFailure p str =
    Assert.True(failure (parse (p .>> eof) str))

[<Fact>]
let ``Simple expression parser`` () =
    AssertParseSuccess expr "1+2"
    AssertParseSuccess expr "1  +   2   "
    AssertParseSuccess expr "(2 +1)"
    AssertParseSuccess expr "( 1  + ( 1+2)+2 +1)  "
    AssertParseFailure expr "1+"
    AssertParseFailure expr "+"
    AssertParseFailure expr "()"

[<Fact>]
let ``Simple parser evaluations`` () =
    AssertExprEvalEqual "1+2" (3.0)

[<Fact>]
let ``Number parsing`` () =
    AssertParseSuccess number "1"
    AssertParseSuccess number "1 "
    AssertParseFailure number " 1"
    AssertParseSuccess number "1.0"
    AssertExprEvalEqual "1" (1.0)
    AssertExprEvalEqual "1.0" (1.0)
    AssertExprEvalEqual "+3.0" (3.0)
    AssertExprEvalEqual "-3.0" (-3.0)
    AssertParseSuccess number "1."
    AssertParseFailure number "."
    AssertParseFailure number ".0"

[<Fact>]
let ``Simple arithmetic expression parser`` () =
    AssertParseSuccess expr "1-2"
    AssertParseSuccess expr "1*2"
    AssertParseSuccess expr "1/2"

[<Fact>]
let ``Simple arithmetic expression evaluation`` () =
    AssertExprEvalEqual "1-2" (-1.0)
    AssertExprEvalEqual "1*2" (2.0)
    AssertExprEvalEqual "1/2" (0.5)
    AssertExprEvalEqual "6-3-2-1" (0.0)
    AssertExprEvalEqual "6-(3-(2-1))" (4.0)
    AssertExprEvalEqual "3*5*7" (3.0*5.0*7.0)
    AssertExprEvalEqual "100/10/2" (5.0)
    AssertExprEvalEqual "100/10/2" (100.0/10.0/2.0)
    AssertExprEvalEqual "2+4*2" (2.0+4.0*2.0)
    AssertExprEvalEqual "2+4/2" (2.0+4.0/2.0)
    AssertExprEvalEqual "2-4*2" (2.0-4.0*2.0)
    AssertExprEvalEqual "2-4/2" (2.0-4.0/2.0)
    AssertExprEvalEqual "(2+4)*2" ((2.0+4.0)*2.0)
    AssertExprEvalEqual "(2+4)/2" ((2.0+4.0)/2.0)
    AssertExprEvalEqual "(2-4)*2" ((2.0-4.0)*2.0)
    AssertExprEvalEqual "(2-4)/2" ((2.0-4.0)/2.0)
    AssertExprEvalEqual "1 % 2" (1.0)
    AssertExprEvalEqual "0 % 2" (0.0)

[<Fact>]
let ``Logical and/or`` () =
    AssertExprEvalEqual "1 && 1" (1.0)
    AssertExprEvalEqual "1 && 0" (0.0)
    AssertExprEvalEqual "0 && 1" (0.0)
    AssertExprEvalEqual "0 && 0" (0.0)
    AssertExprEvalEqual "1 || 1" (1.0)
    AssertExprEvalEqual "1 || 0" (1.0)
    AssertExprEvalEqual "0 || 1" (1.0)
    AssertExprEvalEqual "0 || 0" (0.0)
    AssertExprEvalEqual "0 || 0 || 1" (1.0)
    AssertExprEvalEqual "0 && 0 || 1" (1.0)
    AssertExprEvalEqual "1 && 0 || 1" (1.0)
    AssertExprEvalEqual "1 && 0 && 1" (0.0)

[<Fact>]
let ``Equality`` () =
    AssertExprEvalEqual "1 == 1" (1.0)
    AssertExprEvalEqual "2 == 1+1" (1.0)
    AssertExprEvalEqual "1==1 && 2==2" (1.0)
    AssertExprEvalEqual "0==0" (1.0)
    AssertExprEvalEqual "1==0" (0.0)
    AssertExprEvalEqual "1!=0" (1.0)
    AssertExprEvalEqual "1!=1" (0.0)

[<Fact>]
let ``Relations`` () =
    AssertExprEvalEqual "1 <= 1" (1.0)
    AssertExprEvalEqual "0 <= 1" (1.0)
    AssertExprEvalEqual "-1 <= 1" (1.0)
    AssertExprEvalEqual "2 <= 1" (0.0)
    AssertExprEvalEqual "1 < 1" (0.0)
    AssertExprEvalEqual "0 < 1" (1.0)
    AssertExprEvalEqual "-1 < 1" (1.0)
    AssertExprEvalEqual "2 < 1" (0.0)
    AssertExprEvalEqual "1 >= 1" (1.0)
    AssertExprEvalEqual "0 >= 1" (0.0)
    AssertExprEvalEqual "-1 >= 1" (0.0)
    AssertExprEvalEqual "2 >= 1" (1.0)
    AssertExprEvalEqual "1 > 1" (0.0)
    AssertExprEvalEqual "0 > 1" (0.0)
    AssertExprEvalEqual "-1 > 1" (0.0)
    AssertExprEvalEqual "2 > 1" (1.0)

[<Fact>]
let ``Unary`` () =
    AssertExprEvalEqual "-(1.5)" (-1.5)
    AssertExprEvalEqual "+(1.5)" (1.5)
    AssertExprEvalEqual "!0" (1.0)
    AssertExprEvalEqual "!1" (0.0)

[<Fact>]
let ``Dot parsing`` () =
    AssertParseSuccess expr "sin().2"
    AssertParseFailure expr "sin().2.cos(3)" // Parse 2 as number
    AssertParseSuccess expr "sin().axe().cos(3)"
    AssertParseSuccess expr "sin().axe().flip().3"
    AssertParseFailure expr ".sin(0)"
    AssertParseFailure expr ".sin(0).2"
    AssertParseFailure expr "sin(0)."
    AssertParseFailure expr "sin(0).pos()."
    AssertParseSuccess expr "sin(0).2." // Parse 2 as number
    AssertParseFailure expr "."

[<Fact>]
let ``Function call parsing`` () =
    AssertParseSuccess expr "sin()"
    AssertParseSuccess expr "sin(0)"
    AssertParseFailure expr "sin (0)"
    AssertParseSuccess expr "sin(0)   "
    AssertParseSuccess expr "sin( )"
    AssertParseSuccess expr "sin(  1,   1,   1    ,2,2 , 2,1)"
    AssertParseFailure expr "~sin()"
    AssertParseFailure expr "1sin()"
    AssertParseSuccess expr "_sin()"
    AssertParseSuccess expr "sin1()"
    AssertParseSuccess expr "sin1_()"
    AssertParseSuccess expr "sin1_sin()"
    AssertParseSuccess expr "sin(1+2, 4&&5)"

[<Fact>]
let ``Identifier reference parsing`` () =
    AssertParseSuccess expr "ref"

[<Fact>]
let ``Assign parsing`` () =
    AssertParseSuccess assign "num a = sin()"
    AssertParseSuccess assign "num a = 1"
    AssertParseSuccess assign "num a= 1"
    AssertParseSuccess assign "num a =1"
    AssertParseSuccess assign "num a=1"
    AssertParseFailure assign "num a="
    AssertParseFailure assign "num =1"
    AssertParseFailure assign "=1"

[<Fact>]
let ``Return parsing`` () =
    AssertParseSuccess ret "out 1"
    AssertParseSuccess ret "out sin(1)"
    AssertParseFailure ret " out sin(1)"
    AssertParseFailure ret "ou sin(1)"
    AssertParseFailure ret "out"

[<Fact>]
let ``Section parsing`` () =
    AssertParseSuccess statement "num a = 1"
    AssertParseSuccess statement "out 1"
    AssertParseSuccess section "### test num a=1 out sin(2)"
    AssertParseSuccess section "### test out sin(2)"
    AssertParseSuccess section "### test"
    AssertParseFailure section "###test"

[<Fact>]
let ``Program parsing`` () =
    AssertParseSuccess program ""
    AssertParseSuccess program "### main out 1 ### test num a = sin(1)"

[<Fact>]
let ``Type parsing`` () =
    AssertParseSuccess qualified_type "num"
    AssertParseSuccess qualified_type "num   "
    AssertParseSuccess qualified_type "sample"
    AssertParseSuccess qualified_type "spectrum"
    AssertParseSuccess qualified_type "wavetable"
    AssertParseSuccess qualified_type "const num"
    AssertParseSuccess qualified_type "const num  "
    AssertParseSuccess qualified_type "const    num  "
    AssertParseSuccess qualified_type "const    num"
    AssertParseSuccess qualified_type "const sample"
    AssertParseSuccess qualified_type "const spectrum"
    AssertParseSuccess qualified_type "const wavetable"
    AssertParseFailure qualified_type "const d"
    AssertParseFailure qualified_type "d num"
    AssertParseFailure qualified_type "qqq"
    AssertParseFailure qualified_type "  const sample"
    AssertParseFailure qualified_type "const sample .."
