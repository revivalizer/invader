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

	warnings       "extra"
	floatingpoint  "fast"

	defines { "_CRT_SECURE_NO_WARNINGS" }

	includedirs { "../../trespasser" }

	project "invader_vst"
		kind "SharedLib"
		targetname "invader"


		includedirs { "../../trespasser/external/libs/vstsdk2.4/src" }
		configmap {
			["VSTi Debug"]   = "Debug",
			["VSTi Release"] = "Release"
		}

		files { "*.h", "*.cpp" }
		files { "invader.def" }

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

	include "../../trespasser/external/libs/vstsdk2.4"
	include "../../trespasser/libs/base"
	include "../../trespasser/libs/base_synth"
	include "../synth"
