#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <spdlog/spdlog.h>

#include "AthleteScoreboard.h"
#include "Renderer.h"
#include "ScriptEngine.h"
#include "Window.h"

auto main(const std::int32_t argc, char** const argv) -> int
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        spdlog::error("Failed to initialise SDL2: {}.", SDL_GetError());
        std::cin.get();

        return EXIT_FAILURE;
    }

    if (TTF_Init() != 0)
    {
        spdlog::error("Failed to initialise SDL2_TTF: {}.", TTF_GetError());
        std::cin.get();

        SDL_Quit();

        return EXIT_FAILURE;
    }

    {
        ScriptEngine scriptEngine;

        if (!scriptEngine.IsValid())
        {
            spdlog::error("Failed to load athletes.toml file or it had an error.");
            std::cin.get();

            TTF_Quit();
            SDL_Quit();

            return EXIT_FAILURE;
        }

        AthleteScoreboard athleteScoreboard(scriptEngine);

        if (!athleteScoreboard.DidInitialiseSuccessfully())
        {
            std::cin.get();

            TTF_Quit();
            SDL_Quit();

            return EXIT_FAILURE;
        }

        const Window window(
            Window::Size{
                .width = static_cast<std::uint32_t>(athleteScoreboard.GetWindowHeight() * athleteScoreboard.GetAspectRatio()),
                .height = static_cast<std::uint32_t>(athleteScoreboard.GetWindowHeight()),
            },
            "Athlte Scoreboard"
        );

        if (!window.IsValid())
        {
            spdlog::error("Failed to create window: {}", SDL_GetError());
            std::cin.get();

            TTF_Quit();
            SDL_Quit();

            return EXIT_FAILURE;
        }

        const Renderer renderer(window);

        if (!renderer.IsValid())
        {
            spdlog::error("Failed to create renderer: {}", SDL_GetError());
            std::cin.get();

            TTF_Quit();
            SDL_Quit();

            return EXIT_FAILURE;
        }

        try
        {
            athleteScoreboard.LoadTextCaches(scriptEngine, renderer);
        }
        catch (const std::runtime_error& error)
        {
            spdlog::error("The following error occurred when loading the fonts: {}.", error.what());
            std::cin.get();

            TTF_Quit();
            SDL_Quit();

            return EXIT_FAILURE;
        }
        catch (...)
        {
            spdlog::error("An unknown error occurred when loading the fonts.");
            std::cin.get();

            TTF_Quit();
            SDL_Quit();

            return EXIT_FAILURE;
        }

        std::atomic_bool isRunning = true;
        auto tickCount = std::chrono::steady_clock::now();

        SDL_Event event{ };

        while (isRunning)
        {
            const auto newTickCount = std::chrono::steady_clock::now();
            const auto frameTicks = newTickCount - tickCount;
            tickCount = newTickCount;

            const std::float_t deltaTime = static_cast<std::float_t>(frameTicks.count()) /
                static_cast<std::float_t>(std::chrono::steady_clock::period::den);

            athleteScoreboard.Update(deltaTime);
            athleteScoreboard.Render(renderer);

            renderer.Present();

            while (SDL_PollEvent(&event) == 1)
            {
                switch (event.type)
                {
                case SDL_KEYDOWN:
                    if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                    {
                        isRunning = false;
                    }

                    break;

                case SDL_QUIT:
                    isRunning = false;

                    break;
                }
            }
        }
    }

    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
