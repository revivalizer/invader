module SemanticAnalysisTypes

open ParserTypes;

open System.Collections.Generic;
//type SectionNameMap = Map<string, Section>
open ParserTypes;

type SectionNameMap() =
    inherit Dictionary<string, Section>(HashIdentity.Reference)
    member this.Add' name section =
        base.Add(name, section)
        this

type ParentScope() =
    inherit Dictionary<Statement, Statement>(HashIdentity.Reference)
    member this.Add' child  parent =
        base.Add(child, parent)
        this

type IdentifierRefs() =
    inherit Dictionary<Expr, Statement>(HashIdentity.Reference)
    member this.Add' child  parent =
        base.Add(child, parent)
        this


type IntermediateSemanticAnalysisResult = {
    ast               : Program          option
    sectionNameMap    : SectionNameMap   option
    parentScope       : ParentScope      option
    identifierRefs    : IdentifierRefs   option
}

let defaultIntermediateSemanticAnalysisResult = { ast = None; sectionNameMap = None; parentScope = None; identifierRefs = None; }