project "vstsdk2.4"
	kind "StaticLib"

--	targetdir("build/%{prj.name}/bin/%{cfg.longname}")
--	objdir   ("build/%{prj.name}/obj/%{cfg.longname}")

	warnings "default"

	configmap {
		["VSTi Debug"]   = "Debug",
		["VSTi Release"] = "Release"
	}

	files { "./src/public.sdk/source/vst2.x/*.h", "./src/public.sdk/source/vst2.x/*.cpp" }

	includedirs("./src")

	filter "Debug"
		defines { "DEBUG" }
		flags { "Symbols", "Maps" }
		optimize "Off"

	filter "Release"
		defines { "NDEBUG" }
		optimize "Size"
