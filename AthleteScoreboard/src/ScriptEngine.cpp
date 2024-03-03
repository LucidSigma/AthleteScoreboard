#include "ScriptEngine.h"

#include <cstdint>
#include <exception>
#include <string>

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

ScriptEngine::ScriptEngine()
{
    m_luaState.open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table
    );

    m_luaState.new_usertype<SDL_Colour>(
        "sdl_colour",
        sol::constructors<SDL_Colour(std::uint8_t, std::uint8_t, std::uint8_t, std::uint8_t)>()
    );

    m_luaState.safe_script(
        R"LUA(
            function create_colour(r, g, b)
                return sdl_colour.new(r, g, b, 0xFF)
            end

            function create_color(r, g, b)
                return sdl_colour.new(r, g, b, 0xFF)
            end
        )LUA",
        sol::script_pass_on_error
    );

    char* applicationBasePath = SDL_GetBasePath();

    try
    {
        const sol::protected_function_result scriptResult = m_luaState.safe_script_file(
            applicationBasePath + std::string("data/athletes.lua"),
            sol::script_throw_on_error
        );
    }
    catch (const std::exception& error)
    {
        spdlog::error("Error loading athletes.lua: {}", error.what());
    }
    catch (...)
    {
        spdlog::error("An unknown error occurred when loading athletes.lua.");
    }

    SDL_free(applicationBasePath);
    applicationBasePath = nullptr;

    m_isValid = true;
}
