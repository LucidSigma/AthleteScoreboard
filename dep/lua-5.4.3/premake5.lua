project "lua"
    kind "StaticLib"
    language "C"

    targetdir "%{BUILD_DIRECTORY}/bin/%{cfg.buildcfg}"
    objdir "%{BUILD_DIRECTORY}/bin/obj/%{cfg.buildcfg}"

    files {
        "include/**.h",
        "include/**.hpp",
        "src/**.c",
    }

    includedirs { 
        "%{DependencyIncludes.lua}",
        "%{DependencyIncludes.lua}/lua",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "On"
