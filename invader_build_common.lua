local invader = {}
invader.commontargetdir = "build/%{prj.name}/bin/%{cfg.longname}"
invader.commonobjdir    = "build/%{prj.name}/obj/%{cfg.longname}"

invader.commoncompilerflags   = { "FatalWarnings", "MultiProcessorCompile", "NoFramePointer", "No64BitChecks",  "NoEditAndContinue" }
invader.commonstandaloneflags = { "NoBufferSecurityCheck", "NoExceptions", "NoManifest", "NoRTTI", "NoRuntimeChecks", "OmitDefaultLibrary" }
invader.commonbuildoptions    = { "/wd4725" } -- 4725 is old warning for tan asm func
invader.commonlinkoptions     = { "/NODEFAULTLIB" } -- 4725 is old warning for tan asm func


return invader