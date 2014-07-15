project "invader_synth"
	kind "StaticLib"

--	targetdir("build/%{prj.name}/bin/%{cfg.longname}")
--	objdir   ("build/%{prj.name}/obj/%{cfg.longname}")

	configmap {
		["VSTi Debug"]   = "Debug",
		["VSTi Release"] = "Release"
	}

	pchheader "pch.h"
	pchsource "pch.cpp"

	files { "./vm/*.h", "./vm/*.cpp" }
	files { "./graph/*.h", "./graph/*.cpp" }
	files { "./util/*.h", "./util/*.cpp" }
	files { "./math/levelfollower.h", "./math/levelfollower.cpp" }
	files { "./math/onepolefilter.h", "./math/onepolefilter.cpp" }
	files { "./math/util.h", "./math/util.cpp" }
	files { "./pch.h", "./pch.cpp" }
--	files { "./**.h", "./**.cpp" }
--	files { "invader.def" }
	excludes { "./vm/synthvm*" }

	filter "Debug"
		defines { "DEBUG" }
		flags { "Symbols", "Maps" }
		optimize "Off"

	filter "Release"
		defines { "NDEBUG" }
		optimize "Size"
