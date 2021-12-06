BUILD_DIRECTORY = "%{wks.location}"

workspace "AthleteScoreboard"
    location "build"
    architecture "x86_64"
    startproject "AthleteScoreboard"

    configurations {
        "Debug",
        "Release",
    }

include "dep"

include "AthleteScoreboard"
