local invader = dofile("../invader_build_common.lua")

solution "invader_vst"
	configurations { "VSTi Debug", "VSTi Release" }

	targetdir(invader.commontargetdir)
	objdir   (invader.commonobjdir)

	flags(invader.commoncompilerflags)
	flags(invader.commonstandaloneflags)
	removeflags { "OmitDefaultLibrary" }

	buildoptions(invader.commonbuildoptions)
	linkoptions (invader.commonlinkoptions)
	removelinkoptions { "/NODEFAULTLIB" }

	warnings       "extra"
	floatingpoint  "fast"

	defines { "_CRT_SECURE_NO_WARNINGS" }

	includedirs { "../../64klibs" }
	includedirs { "$(SolutionDir)/.." } -- for config.h

	newoption {
	   trigger     = "vstdir",
	   value       = "path",
	   description = "Output directory for the compiled executable"
	}

	if (_OPTIONS.vstdir==nil) then
		print("You must specify a target VST directory with --vstdir=PATH")
		os.exit()
	end

	location "build"

	project "invader_vst"
		kind "SharedLib"
		targetname "invader"

		postbuildcommands { "copy \"$(TargetPath)\" \"".._OPTIONS.vstdir.."\" " }		
		debugcommand "$(TargetDir)\\$(TargetName).exe" -- this should launch SAVIhost which should be placed in the target folders, and be named the same as the target file (invader.exe)

		includedirs { "libs/vstsdk2.4/src" }
		configmap {
			["VSTi Debug"]   = "Debug",
			["VSTi Release"] = "Release"
		}

		files { "*.h", "*.cpp" }
		files { "invader.def" }
		files { "TODO.txt" }

		links { "invader_synth" }
		links { "vstsdk2.4" }
		links { "base" }
		links { "base_synth" }

		filter "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "Maps" }
			optimize "Off"

		filter "Release"
			defines { "NDEBUG" }
			optimize "Size"

	include "libs/vstsdk2.4"
	include "../../64klibs/base"
	include "../../64klibs/base_synth"
	include "../synth"


-- Ralphs make: in invader/vst: premake5 vs2012 --vstdir="C:\Program Files (x86)\Steinberg\VstPlugins"