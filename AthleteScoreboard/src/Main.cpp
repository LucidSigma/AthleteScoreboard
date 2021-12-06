#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <sol/sol.hpp>

#include "ScriptEngine.h"

struct Athlete final
{
    std::string name;

    std::uint32_t currentScore;
    std::int32_t pointsToAdd;

    bool isEliminated;

};

[[nodiscard]] inline auto operator >(const Athlete& lhs, const Athlete& rhs) noexcept -> bool
{
    if (lhs.currentScore == rhs.currentScore)
    {
        return lhs.name < rhs.name;
    }

    return lhs.currentScore > rhs.currentScore;
}

auto operator <<(std::ostream& outputStream, const Athlete& athlete) -> std::ostream&
{
    outputStream << athlete.name << ": ";

    if (athlete.isEliminated)
    {
        outputStream << "ELIMINATED";

        return outputStream;
    }

    outputStream << "(" << athlete.currentScore << " -> " << (athlete.currentScore + athlete.pointsToAdd) << ")";

    return outputStream;
}

auto main(const int argc, char** const argv) -> int
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        spdlog::error("Failed to initialise SDL2: {}", SDL_GetError());
        std::cin.get();

        return EXIT_FAILURE;
    }

    ScriptEngine scriptEngine;

    if (!scriptEngine.IsValid())
    {
        spdlog::error("Failed to load athletes.toml file or it had an error.");
        std::cin.get();

        SDL_Quit();

        return EXIT_FAILURE;
    }

    std::vector<Athlete> athletes{ };

    for (const auto& [athleteName, athleteData] : scriptEngine.Get<sol::table>("ATHLETES"))
    {
        const bool isAthleteEliminated = athleteData.as<sol::table>()["is_eliminated"];

        athletes.push_back(
            Athlete{
                .name = athleteName.as<std::string>(),
                .currentScore = athleteData.as<sol::table>()["current_score"],
                .pointsToAdd = isAthleteEliminated ? 0 : athleteData.as<sol::table>()["points_to_add"],
                .isEliminated = isAthleteEliminated,
            }
        );
    }

    std::ranges::sort(athletes, std::greater());

    for (const auto& athlete : athletes)
    {
        std::cout << athlete << "\n";
    }

    const std::float_t aspectRatio = scriptEngine["DIMENSIONS"]["aspect_ratio"];
    const std::uint32_t barHeight = scriptEngine["DIMENSIONS"]["bar_height"];
    const std::uint32_t distanceBetweenBars = scriptEngine["DIMENSIONS"]["distance_between_bars"];

    const std::float_t windowHeight = static_cast<std::float_t>(static_cast<std::uint32_t>(athletes.size()) * (barHeight + distanceBetweenBars) + distanceBetweenBars);

    SDL_Window* window = SDL_CreateWindow(
        "Athlete Scoreboard",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<std::int32_t>(windowHeight * aspectRatio),
        static_cast<std::int32_t>(windowHeight),
        SDL_WINDOW_SHOWN
    );

    if (window == nullptr)
    {
        spdlog::error("Failed to create window: {}", SDL_GetError());
        std::cin.get();

        SDL_Quit();

        return EXIT_FAILURE;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr)
    {
        spdlog::error("Failed to create renderer: {}", SDL_GetError());
        std::cin.get();

        SDL_DestroyWindow(window);
        window = nullptr;

        SDL_Quit();

        return EXIT_FAILURE;
    }

    const SDL_Colour backgroundColour = scriptEngine["COLOURS"]["background"];

    std::atomic_bool isRunning = true;
    SDL_Event event{ };

    while (isRunning)
    {
        SDL_SetRenderDrawColor(renderer, backgroundColour.r, backgroundColour.g, backgroundColour.b, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_RenderPresent(renderer);

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

    SDL_DestroyRenderer(renderer);
    renderer = nullptr;

    SDL_DestroyWindow(window);
    window = nullptr;

    SDL_Quit();

    spdlog::info("Press enter to exit.");
    std::cin.get();

    return EXIT_SUCCESS;
}
