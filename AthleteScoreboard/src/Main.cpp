#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <spdlog/spdlog.h>
#include <sol/sol.hpp>

#include "ScriptEngine.h"

struct Athlete final
{
    std::string name;
    SDL_Colour colour;

    std::uint32_t originalScore;
    std::uint32_t currentScore;
    std::uint32_t newScore;
    std::int32_t pointsToAdd;

    bool isEliminated;

    std::uint32_t currentRanking;
    std::int32_t originalPosition;
    std::int32_t currentPosition;

    std::uint32_t newRanking;
    std::int32_t newPosition;
};

[[nodiscard]] inline auto operator >(const Athlete& lhs, const Athlete& rhs) noexcept -> bool
{
    if (lhs.currentScore == rhs.currentScore)
    {
        return lhs.name < rhs.name;
    }

    return lhs.currentScore > rhs.currentScore;
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
                .colour = athleteData.as<sol::table>()["colour"],
                .currentScore = athleteData.as<sol::table>()["current_score"],
                .pointsToAdd = isAthleteEliminated ? 0 : athleteData.as<sol::table>()["points_to_add"],
                .isEliminated = isAthleteEliminated,
            }
        );
    }

    std::ranges::sort(athletes, std::greater());

    const std::float_t aspectRatio = scriptEngine["DIMENSIONS"]["aspect_ratio"];
    const std::uint32_t barHeight = scriptEngine["DIMENSIONS"]["bar_height"];
    const std::uint32_t distanceBetweenBars = scriptEngine["DIMENSIONS"]["distance_between_bars"];

    const std::float_t windowHeight = static_cast<std::float_t>(static_cast<std::uint32_t>(athletes.size()) * (barHeight + distanceBetweenBars) + distanceBetweenBars);

    std::int32_t yOffset = static_cast<std::int32_t>(distanceBetweenBars);

    for (std::size_t i = 0u; i < athletes.size(); ++i)
    {
        athletes[i].originalScore = athletes[i].currentScore;
        athletes[i].currentRanking = static_cast<std::uint32_t>(i);
        athletes[i].currentPosition = yOffset;
        athletes[i].originalPosition = athletes[i].currentPosition;

        yOffset += static_cast<std::int32_t>(barHeight + distanceBetweenBars);
    }

    std::vector<Athlete> newAthletes = athletes;

    for (auto& athlete : newAthletes)
    {
        athlete.currentScore += athlete.pointsToAdd;
        athlete.pointsToAdd = 0;
    }

    std::ranges::sort(newAthletes, std::greater());

    for (auto& athlete : athletes)
    {
        const auto newAthleteLocation = std::ranges::find_if(
            newAthletes,
            [&targetName = athlete.name](const Athlete& athlete) -> bool { return athlete.name == targetName; }
        );

        const std::size_t newAthleteIndex = newAthleteLocation - std::cbegin(newAthletes);

        athlete.newScore = newAthleteLocation->currentScore;
        athlete.newRanking = static_cast<std::uint32_t>(newAthleteIndex + 1u);
        athlete.newPosition = static_cast<std::int32_t>(static_cast<std::uint32_t>(newAthleteIndex) * (barHeight + distanceBetweenBars) + distanceBetweenBars);
    }

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
    const SDL_Colour sidebarColour = scriptEngine["COLOURS"]["sidebar"];

    std::atomic_bool isRunning = true;
    SDL_Event event{ };

    std::float_t counter = 0.0f;

    while (isRunning)
    {
        if (counter <= 1.0f)
        {
            for (auto& athlete : athletes)
            {
                athlete.currentScore = std::lerp(athlete.originalScore, athlete.newScore, counter);
                athlete.currentPosition = std::lerp(athlete.originalPosition, athlete.newPosition, counter);
            }

            counter += 0.00005f;
        }

        SDL_SetRenderDrawColor(renderer, backgroundColour.r, backgroundColour.g, backgroundColour.b, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, sidebarColour.r, sidebarColour.g, sidebarColour.b, SDL_ALPHA_OPAQUE);

        const SDL_Rect sidebar{
            .x = 0,
            .y = 0,
            .w = 400,
            .h = static_cast<std::int32_t>(windowHeight),
        };

        SDL_RenderFillRect(
            renderer,
            &sidebar
        );

        for (const auto& athlete : athletes)
        {
            const SDL_Rect currentAthleteBar{
                .x = sidebar.w,
                .y = athlete.currentPosition,
                .w = static_cast<std::int32_t>(athlete.currentScore) * 10 + 5,
                .h = static_cast<std::int32_t>(barHeight),
            };

            SDL_SetRenderDrawColor(renderer, athlete.colour.r, athlete.colour.g, athlete.colour.b, SDL_ALPHA_OPAQUE);
            SDL_RenderFillRect(renderer, &currentAthleteBar);
        }

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

    return EXIT_SUCCESS;
}
