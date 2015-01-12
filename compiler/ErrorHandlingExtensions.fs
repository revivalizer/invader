module ErrorHandlingExtensions

open ErrorHandling

let binaryRail addSuccess res1 res2 = 
    match res1,res2 with
    | Success s1,Success s2 -> Success (addSuccess s1 s2)
    | Failure f1,Success _  -> Failure f1
    | Success _ ,Failure f2 -> Failure f2
    | Failure f1,Failure f2 -> Failure (f1 + "; " + f2)

let success f =
    match f with
    | Success(_) -> true
    | Failure(_) -> false

let failure f =
    not (success f)

