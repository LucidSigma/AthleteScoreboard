project "AthleteScoreboard"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    compileas "Module"

    targetdir "%{BUILD_DIRECTORY}/bin/%{cfg.buildcfg}"
    objdir "%{BUILD_DIRECTORY}/bin/obj/%{cfg.buildcfg}"

    files {
        "modules/**.ixx",
        "src/**.cpp",
    }

    vpaths {
        ["*"] = {
            "modules/**",
            "src/**",
        },
    }

    includedirs {
        "modules",
        "%{DependencyIncludes.lua}",
        "%{DependencyIncludes.SDL2}",
        "%{DependencyIncludes.SDL2}/SDL2",
        "%{DependencyIncludes.SDL2_ttf}",
        "%{DependencyIncludes.sol2}",
        "%{DependencyIncludes.spdlog}",
    }

    libdirs {
        "%{DependencySources.SDL2}",
        "%{DependencySources.SDL2_ttf}",
    }

    links {
        "lua",
        "SDL2",
        "SDL2main",
        "SDL2_ttf",
        "spdlog",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "On"
