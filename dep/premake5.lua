DEPENDENCIES = {
    lua = {
        version = "5.4.3",
        has_premake_file = true,
    },
    SDL2 = {
        version = "2.0.18",
        is_precompiled = true,
    },
    SDL2_ttf = {
        version = "2.0.15",
        is_precompiled = true,
    },
    sol2 = {
        version = "3.2.2",
    },
    spdlog = {
        version = "1.9.2",
        has_premake_file = true,
    },
}

DEPENDENCIES_LOCATION = "%{wks.location}/../dep/"

DependencyIncludes = { }
DependencySources = { }
local dependencies_to_compile = { }

for dependency, data in pairs(DEPENDENCIES) do
    DependencyIncludes[dependency] = DEPENDENCIES_LOCATION .. dependency .. "-" .. data.version .. "/include"

    if data.is_precompiled then
        DependencySources[dependency] = DEPENDENCIES_LOCATION .. dependency .. "-" .. data.version .. "/lib"
    end

    if data.has_premake_file then
        dependencies_to_compile[dependency] = data.version
    end
end

group "Dependencies"
    for dependency, version in pairs(dependencies_to_compile) do
        include (dependency .. "-" .. version)
    end
group ""
