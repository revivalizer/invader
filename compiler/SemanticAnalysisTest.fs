module SemanticAnalysisTests

open ErrorHandling;
open ErrorHandlingExtensions;
open Xunit;
open SemanticAnalysisTypes;
open SemanticAnalysisLogic;
open ParserLogic;
open ParserTypes;

let pc = parse program >> bind checkSectionCount

let AssertCheckSuccess check str =
    Assert.True(success ((parse program >> bind check) str))

let AssertCheckFailure check str =
    Assert.True(failure ((parse program >> bind check) str))

[<Fact>]
let ``Check section count`` () =
    AssertCheckSuccess checkSectionCount "### t   ### r"
    AssertCheckFailure checkSectionCount "### t"
    AssertCheckFailure checkSectionCount ""

[<Fact>]
let ``Check first section is global`` () =
    AssertCheckSuccess checkFirstSectionIsGlobal "### global   ### r"
    AssertCheckSuccess checkFirstSectionIsGlobal "### global"
    AssertCheckFailure checkFirstSectionIsGlobal "### t ### r"

[<Fact>]
let ``Check last section is master`` () =
    AssertCheckSuccess checkLastSectionIsMaster "### master"
    AssertCheckSuccess checkLastSectionIsMaster "### r ### master"
    AssertCheckFailure checkLastSectionIsMaster "### master ### hello"
    AssertCheckFailure checkLastSectionIsMaster "### hello"

[<Fact>]
let ``Return statements must be last statement in section`` () =
    AssertCheckSuccess checkIllegalOutputInSections "### master num a = 1 num b = 2 out 1"
    AssertCheckFailure checkIllegalOutputInSections "### master num a = 1 num b = 2 out 1 num c = 3"
    AssertCheckSuccess checkIllegalOutputInSections "### master num a = 1 num b = 2 out 1 ### r num d = 1 out sin(2)"
    AssertCheckFailure checkIllegalOutputInSections "### master num a = 1 num b = 2 out 1 ### r out 1 num f = a"

[<Fact>]
let ``Correct returns from sections`` () =
    AssertCheckSuccess checkCorrectSectionOutputs "### master num a = 1 num b = 2 out 1"
    AssertCheckSuccess checkCorrectSectionOutputs "### master num a = 1 num b = 2 out 1 ### q num a = 1 num b = 2 out 1"
    AssertCheckSuccess checkCorrectSectionOutputs "### master num a = 1 num b = 2 out 1 ### global num f = 2"
    AssertCheckSuccess checkCorrectSectionOutputs "### master num a = 1 num b = 2 out 1 ### q num a = 1 num b = 2 out 1 ### const_global num q = 1 num r = 4"
    AssertCheckFailure checkCorrectSectionOutputs "### global num a = 1 out 1"
    AssertCheckFailure checkCorrectSectionOutputs "### global out 1"
    AssertCheckFailure checkCorrectSectionOutputs "### const_global num a = 1 out 1"
    AssertCheckFailure checkCorrectSectionOutputs "### const_global out 1"

[<Fact>]
let ``Duplicate finding`` () =
    Assert.True ((duplicatesUnique [1;2;2]) = [2])
    Assert.True ((duplicatesUnique [1;2;2;1]) = [1;2])
    Assert.True ((duplicatesUnique []) = [])
    Assert.True ((duplicatesUnique [1;2;3;4;5;6;7]) = [])

[<Fact>]
let ``Unique section names`` () =
    AssertCheckSuccess checkUniqueSectionNames "### c1 ### c2"
    AssertCheckSuccess checkUniqueSectionNames "### c1 ### c2 ### master ### global"
    AssertCheckFailure checkUniqueSectionNames "### c1 ### c2 ### c1 ### c2"
    AssertCheckFailure checkUniqueSectionNames "### c1 ### c1"

[<Fact>]
let ``Parent scope generation`` () =
    // Generate dummy sections
    let dp = new FParsec.Position("", 0L, 0L, 0L) // dummy position
    let s1 = Statement.Assign(dp, Varying(dp, Num), "a", Expr.Number(dp, 1.0))
    let s2 = Statement.Assign(dp, Varying(dp, Num), "b", Expr.Number(dp, 2.0))
    let s3 = Statement.Assign(dp, Varying(dp, Num), "c", Expr.Number(dp, 3.0))
    let s4 = Statement.Assign(dp, Varying(dp, Num), "i", Expr.Number(dp, 1.0))
    let s5 = Statement.Assign(dp, Varying(dp, Num), "j", Expr.Number(dp, 2.0))
    let s6 = Statement.Assign(dp, Varying(dp, Num), "k", Expr.Number(dp, 3.0))
    let sec1 = Section(dp, "r", [s1; s2; s3])
    let sec2 = Section(dp, "t", [s4; s5; s6])
    let prog = Program(dp, [sec1; sec2])

    // Generate scope lists
    let gen = toIntermediateSemanticAnalysisResult >> bind generateLocalSectionScopeLists
    let r = gen prog

    // Tests
    match r with
    | Success(r) ->
        match r.parentScope with
        | Some(parentScope) ->
                               Assert.True(parentScope.Count = 4)
                               Assert.True(parentScope.ContainsKey s2 && (parentScope.Item s2) = s1)
                               Assert.True(parentScope.ContainsKey s3 && (parentScope.Item s3) = s2)
                               Assert.True(parentScope.ContainsKey s5 && (parentScope.Item s5) = s4)
                               Assert.True(parentScope.ContainsKey s6 && (parentScope.Item s6) = s5)
                               Assert.False(parentScope.ContainsKey s1)
                               Assert.False(parentScope.ContainsKey s4)

        | None              -> Assert.True(false, "No parentscope generated")
    | Failure(f)            -> Assert.True(false, "Failure: " + f)

[<Fact>]
let ``Const extraction`` () =
    // Generate dummy sections
    let dp = new FParsec.Position("", 0L, 0L, 0L) // dummy position
    let s1 = Statement.Assign(dp, Const(dp, Num), "a", Expr.Number(dp, 1.0))
    let s2 = Statement.Assign(dp, Const(dp, Num), "b", Expr.Number(dp, 2.0))
    let s3 = Statement.Assign(dp, Varying(dp, Num), "c", Expr.Number(dp, 3.0))
    let s4 = Statement.Assign(dp, Varying(dp, Num), "i", Expr.Number(dp, 1.0))
    let s5 = Statement.Assign(dp, Varying(dp, Num), "j", Expr.Number(dp, 2.0))
    let s6 = Statement.Assign(dp, Const(dp, Num), "k", Expr.Number(dp, 3.0))
    let sec1 = Section(dp, "r", [s1; s2; s3])
    let sec2 = Section(dp, "t", [s4; s5; s6])
    let prog = Program(dp, [sec1; sec2])

    // Generate scope lists
    let r = (bind toIntermediateSemanticAnalysisResult) (Success prog) |> (bind generateConstSection)
    let nsec1 = Section(dp, "const_global", [s1; s2; s6])
    let nsec2 = Section(dp, "r", [s3])
    let nsec3 = Section(dp, "t", [s4; s5])
    let expectedprog = Program(dp, [nsec1; nsec2; nsec3])

    // Tests
    match r with
    | Success(r) ->
        match r.ast with
        | Some(ast) ->
                               Assert.True(List.length (sections ast) = 3)
                               let equal = (ast = expectedprog)
                               Assert.True(equal)

        | None              -> Assert.True(false, "No AST generate")
    | Failure(f)            -> Assert.True(false, "Failure: " + f)

[<Fact>]
let ``Identifier refs`` () =
    // Generate dummy sections
    let dp = new FParsec.Position("", 0L, 0L, 0L) // dummy position
    let refa = Expr.IdentifierRef(dp, "a")
    let refb = Expr.IdentifierRef(dp, "b")
    let refc = Expr.IdentifierRef(dp, "c")
    let refi = Expr.IdentifierRef(dp, "i")
    let refj = Expr.IdentifierRef(dp, "j")
    let refk = Expr.IdentifierRef(dp, "k")
    let refr = Expr.IdentifierRef(dp, "r")
    let reft = Expr.IdentifierRef(dp, "t")
    let s1 = Statement.Assign(dp, Const(dp, Num), "a",   Expr.Number(dp, 1.0))
    let s2 = Statement.Assign(dp, Const(dp, Num), "b",   refa)
    let s3 = Statement.Assign(dp, Varying(dp, Num), "c", refb)
    let s4 = Statement.Assign(dp, Varying(dp, Num), "i", Expr.Number(dp, 1.0))
    let s5 = Statement.Assign(dp, Varying(dp, Num), "j", refi)
    let s6 = Statement.Assign(dp, Const(dp, Num), "k",   refj)
    let sec1 = Section(dp, "r", [s1; s2; s3])
    let sec2 = Section(dp, "t", [s4; s5; s6])
    let prog = Program(dp, [sec1; sec2])

    // Generate scope lists
    let r = Success prog |> bind toIntermediateSemanticAnalysisResult |> bind generateLocalSectionScopeLists |> bind generateConstSection |> bind generateIdentifierRefs

    // Tests
    match r with
    | Success(r) ->
        match r.ast,r.identifierRefs with
        | Some(ast),Some(identifierRefs) ->
                               Assert.True(identifierRefs.Item refa = s1)
                               Assert.True(identifierRefs.Item refb = s2)
                               Assert.True(identifierRefs.Item refi = s4)
                               Assert.True(identifierRefs.Item refj = s5)

        | _                 -> Assert.True(false, "No AST or identifierRefs")
    | Failure(f)            -> Assert.True(false, "Failure: " + f)

