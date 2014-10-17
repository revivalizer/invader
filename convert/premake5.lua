local invader = dofile("../invader_build_common.lua")

solution "convert"
	configurations { "Debug", "Release" }

	targetdir(invader.commontargetdir)
	objdir   (invader.commonobjdir)

	flags(invader.commoncompilerflags)
	flags(invader.commoncompilerflags)
--	flags(invader.commonstandaloneflags)
	removeflags { "OmitDefaultLibrary" }

	buildoptions(invader.commonbuildoptions)
	linkoptions (invader.commonlinkoptions)
	removelinkoptions { "/NODEFAULTLIB" }

	warnings       "extra"
	floatingpoint  "fast"

	defines { "_CRT_SECURE_NO_WARNINGS" }

	includedirs { "../synth" }
	includedirs { "../../trespasser" }
	includedirs { "../../trespasser/external/libs/zlib/src" }
	includedirs { "../../trespasser/external/libs/tinyxml/src" }
	includedirs { "../../trespasser/external/libs/minizip/src" }

	location "build"

	project "convert"
		kind "ConsoleApp"

		buildoptions { "/wd4244" }

		pchheader "pch.h"
		pchsource "pch.cpp"

		files { "*.h", "*.cpp" }

		links { "minizip" }
		links { "tinyxml" }
		links { "zlib" }
		links { "invader_synth" }

		filter "Debug"
			defines { "DEBUG" }
			flags { "Symbols", "Maps" }
			optimize "Off"

		filter "Release"
			defines { "NDEBUG" }
			optimize "Size"

	include "../../trespasser/external/libs/minizip"
	include "../../trespasser/external/libs/tinyxml"
	include "../../trespasser/external/libs/zlib"
	include "../synth"
