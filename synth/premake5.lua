project "invader_synth"
	kind "StaticLib"

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
	files { "./math/resampler2x.h", "./math/resampler2x.cpp" }
	files { "./math/adsrenvelope.h", "./math/adsrenvelope.cpp" }
	files { "./math/spectrum.h", "./math/spectrum.cpp" }
	files { "./math/shapingtransform.h" }
	files { "./math/onepolefilterfast.h" }
	files { "./math/util.h", "./math/util.cpp" }
	files { "./math/fft.h", "./math/fft.cpp" }
	files { "./math/filterzd24.*" }
	files { "./nodes/adsrgain.*" }
	files { "./nodes/adsrmod.*" }
	files { "./nodes/wavetableosc.h", "./nodes/wavetableosc.cpp" }
	files { "./nodes/filter.*" }
	files { "./nodes/compressor.*" }
	files { "./nodes/reverb.*" }
	files { "./song/song.*" }
	files { "./song/renderer.*" }
	files { "./pch.h", "./pch.cpp" }
	files { "./synth.h" }
	files { "./wavetable/*" }
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
