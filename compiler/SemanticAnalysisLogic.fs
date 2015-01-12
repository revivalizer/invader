module SemanticAnalysisLogic

open ParserTypes;
open ErrorHandling;
open SemanticAnalysisTypes;

let sections (ast : Program) =
    let (Program(_, sections)) = ast
    sections

let statements (section : Section) =
    let (Section(pos, name, statements)) = section
    statements

let name (section : Section) =
    let (Section(pos, name, statements)) = section
    name

let lastElement ls =
    List.reduce (fun _ i -> i) ls

let firstElement (ls : 'a list) = ls.Head

let checkSectionCount (ast : Program) =
    match (sections ast).Length with
    | length when length>=2 -> Success ast
    | _                     -> Failure "'global' section and 'master' section are required"

let checkFirstSectionIsGlobal (ast : Program) =
    match firstElement (sections ast) with
    | Section(_, "global", _) -> Success ast
    | _                       -> Failure "First section must be 'global'."

let checkLastSectionIsMaster (ast : Program) =
    match lastElement (sections ast) with
    | Section(_, "master", _) -> Success ast
    | _                       -> Failure "Last section must be 'master'."

let rec foldResult f seed list =
    match list with
    | head :: tail -> match f head with
                      | Success(_) -> (bind (foldResult f seed)) (Success tail)
                      | Failure f  -> Failure f
    | []           -> Success seed

let checkIllegalOutputInSections (ast : Program) =
    let rec checkStatement list =
        match list with
        | hd :: tl when tl=[]   -> Success []
        | hd :: tl              -> match hd with
                                            | Assign(_)      -> (bind checkStatement) (Success tl)
                                            | Return(pos, _) -> Failure (sprintf "Unexpected return at %A" pos)
        | []                    -> Success []
    let sectionCheck section =
        (bind checkStatement) (Success (statements section))
    foldResult sectionCheck (Success ast) (sections ast)

let mustOutput section =
    match name section with
    | "global" | "const_global" -> false
    | _                         -> true

let checkCorrectSectionOutputs (ast : Program) =
    let sectionCheck section =
        match mustOutput section,lastElement (statements section) with
        | true,Assign(pos, _, _, _) -> Failure (sprintf "Expected output statement at %A" pos)
        | false,Return(pos,_)    -> Failure (sprintf "Unexpected output statement at %A" pos)
        | _                      -> Success true
    foldResult sectionCheck (Success ast) (sections ast)

let duplicatesUnique list =
    let check ((acc : Set<'a>), (duplicates : Set<'a>)) sectionName =
        match Set.contains sectionName acc with
        | true  -> (acc, (duplicates.Add sectionName))
        | false -> ((acc.Add sectionName), duplicates)
    list |> List.fold check (Set.empty, Set.empty) |> snd |> Set.toList

let checkUniqueSectionNames (ast : Program) =
    let duplicates = sections ast
                     |> List.map name
                     |> duplicatesUnique
    match List.length duplicates with
    | 0 -> Success ast
    | 1 -> Failure ("The section '" + duplicates.Head + "' defined more than once.")
    | _ -> Failure ("The following sections were defined more than once: " + (duplicates |> List.fold (fun a i -> a + ", " + i) ""))

let toIntermediateSemanticAnalysisResult ast =
    Success { defaultIntermediateSemanticAnalysisResult with ast = Some(ast) }

// It's actually to early for this, sections will change!
let generateSectionNameMap result =
    let addToMap (nameMap : SectionNameMap) (section : Section) =
        nameMap.Add' (name section) section
    let makeNameMap = List.fold addToMap (new SectionNameMap())
    match result.ast with
       | Some(ast) -> Success { result with sectionNameMap = Some(makeNameMap (sections ast))}
       | None      -> Failure "No AST available in generateSectionNameMap"

let generateLocalSectionScopeLists result =
    let parentScope = new ParentScope()
    let rec generate statements =
        match statements with
        | hd::tail when tail<>[] -> parentScope.Add(tail.Head, hd)
                                    generate tail
        | _                      -> ()
    let getScopeStatements globalScope section =
        let mustChainGlobalScope = mustOutput // same sections
        match mustChainGlobalScope section with
        | true -> globalScope :: (statements section)
        | false -> statements section
    match result.ast with
       | Some(ast) -> sections ast
                      |> List.map statements
                      |> List.iter generate
                      Success {result with parentScope = Some(parentScope)}
       | None      -> Failure "No AST available in generateLocalSectionScope"

let generateGlobalScopeList result =
    let addGlobalScope globalScope (parentScope : ParentScope) section =
        parentScope.Add(firstElement (statements section), globalScope)
    let onlyInstrumentOrMaster section =
        match section with
        | Section(_, id, _) when id<>"global" -> true
        | _                                   -> false
    let onlyGlobal s = not (onlyInstrumentOrMaster s)
    let globalScope ast =
        sections ast
        |> List.filter onlyGlobal
        |> List.head
        |> statements
        |> lastElement
    match result.ast,result.parentScope with
        | Some(ast),Some(parentScope)
            ->  sections ast
                |> List.filter onlyInstrumentOrMaster
                |> List.iter (addGlobalScope (globalScope ast) parentScope)
                Success result
        | _
            ->  Failure "AST or parentScope not available in generateLocalSectionScope"

let generateConstSection result =
    // Filter statements into const assignment list and other list
    let groupStatementsByType (constAssignments, remainingStatements) statement = 
        match statement with
        | Statement.Assign(_, Const(_), _, _)   -> (statement :: constAssignments, remainingStatements)
        | _                                     -> (constAssignments, statement :: remainingStatements)
    // Return const assignments, and create new section without them
    let extractConstAssignments section =
        let (Section(pos, id, statements))          = section
        let (constAssignments, remainingStatements) = List.fold groupStatementsByType ([], []) statements
        let transformedSection                      = Section(pos, id, List.rev remainingStatements)
        (constAssignments, transformedSection)
    // Create lists of const assignments and transformated sections
    let transformSection (constStatements, sections) section =
        let (extractedStatements, transformedSection) = extractConstAssignments section
        (extractedStatements @ constStatements, transformedSection :: sections)
    match result.ast with
    | Some(ast) -> let (Program(pos, sections))               = ast
                   let (constStatements, transformedSections) = List.fold transformSection ([], []) sections
                   let constSection                           = Section(pos, "const_global", List.rev constStatements) // pos is not really right, we should use dummy pos
                   let newProgram                             = Program(pos, constSection :: (List.rev transformedSections))
                   Success { result with ast = Some(newProgram)}
    | None      -> Failure "No AST available in generateConstSection"

let generateIdentifierRefs result =
    Success {result with identifierRefs = Some(new IdentifierRefs()) } 

(*let checkUniqueSectionNames (ast : Program) =
    let check names section =
        let name = name section
        match Set.contains name names with
        | false  -> Success (Set.add name names)
        | true   -> Failure name
    List.fold (bind check) (Success Set.empty) (sections ast)

let checkUniqueSectionNames (ast : Program) =
    let mutable names = Set.empty
    let sectionCheck section =
        let name = name section
        match Set.contains name names with
        | true  -> Failure name
        | false -> names = Set.add name names
                   Success true
    foldResult sectionCheck (Success ast) (sections ast)
   *) 