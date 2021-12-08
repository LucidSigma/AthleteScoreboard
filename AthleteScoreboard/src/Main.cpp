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

#include "Renderer.h"
#include "ScriptEngine.h"
#include "TextCache.h"
#include "Utility.h"
#include "Window.h"

struct Athlete final
{
    std::string name;
    SDL_Colour colour;

    std::int32_t pointsToAdd;
    bool isEliminated;

    std::uint32_t originalScore;
    std::float_t currentScore;
    std::uint32_t newScore;

    std::uint32_t currentRanking;
    std::uint32_t newRanking;

    std::int32_t originalPosition;
    std::float_t currentPosition;
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

auto main(const std::int32_t argc, char** const argv) -> int
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        spdlog::error("Failed to initialise SDL2: {}", SDL_GetError());
        std::cin.get();

        return EXIT_FAILURE;
    }

    if (TTF_Init() != 0)
    {
        spdlog::error("Failed to initialise SDL2_TTF: {}", TTF_GetError());
        std::cin.get();

        SDL_Quit();

        return EXIT_FAILURE;
    }

    ScriptEngine scriptEngine;

    if (!scriptEngine.IsValid())
    {
        spdlog::error("Failed to load athletes.toml file or it had an error.");
        std::cin.get();

        TTF_Quit();
        SDL_Quit();

        return EXIT_FAILURE;
    }

    std::vector<Athlete> athletes{ };

    // Load athletes from Lua file.
    for (const auto& [athleteName, athleteData] : scriptEngine.Get<sol::table>("ATHLETES"))
    {
        const bool isAthleteEliminated = athleteData.as<sol::table>()["is_eliminated"];

        athletes.push_back(
            Athlete{
                .name = athleteName.as<std::string>(),
                .colour = athleteData.as<sol::table>()["colour"],
                .pointsToAdd = isAthleteEliminated ? 0 : athleteData.as<sol::table>()["points_to_add"],
                .isEliminated = isAthleteEliminated,
                .currentScore = athleteData.as<sol::table>()["current_score"],
            }
        );
    }

    std::ranges::sort(athletes, std::greater());

    // Load dimensions from Lua file.
    const std::float_t aspectRatio = scriptEngine["DIMENSIONS"]["aspect_ratio"];
    const std::uint32_t barHeight = scriptEngine["DIMENSIONS"]["bar_height"];
    const std::uint32_t distanceBetweenBars = scriptEngine["DIMENSIONS"]["distance_between_bars"];

    // Determine window height based on number of athletes.
    const std::float_t windowHeight = static_cast<std::float_t>(static_cast<std::uint32_t>(athletes.size()) * (barHeight + distanceBetweenBars) + distanceBetweenBars);

    // Calculate the athletes' vertical positions.
    std::int32_t yOffset = static_cast<std::int32_t>(distanceBetweenBars);

    for (std::size_t i = 0u; i < athletes.size(); ++i)
    {
        athletes[i].originalScore = static_cast<std::uint32_t>(athletes[i].currentScore);
        athletes[i].currentRanking = static_cast<std::uint32_t>(i);
        athletes[i].currentPosition = static_cast<std::float_t>(yOffset);
        athletes[i].originalPosition = static_cast<std::int32_t>(athletes[i].currentPosition);

        yOffset += static_cast<std::int32_t>(barHeight + distanceBetweenBars);
    }

    // Determine the athletes' new scores.
    std::vector<Athlete> newAthletes = athletes;
    std::uint32_t maxScore = 0u;

    for (auto& athlete : newAthletes)
    {
        athlete.currentScore += static_cast<std::float_t>(athlete.pointsToAdd);
        athlete.pointsToAdd = 0;

        maxScore = std::max(maxScore, static_cast<std::uint32_t>(athlete.currentScore));
    }

    std::ranges::sort(newAthletes, std::greater());

    // Determine the athletes' new rankings and positions.
    for (auto& athlete : athletes)
    {
        const auto newAthleteLocation = std::ranges::find_if(
            newAthletes,
            [&targetName = athlete.name](const Athlete& athlete) -> bool { return athlete.name == targetName; }
        );

        const std::size_t newAthleteIndex = newAthleteLocation - std::cbegin(newAthletes);

        athlete.newScore = static_cast<std::uint32_t>(newAthleteLocation->currentScore);
        athlete.newRanking = static_cast<std::uint32_t>(newAthleteIndex + 1u);
        athlete.newPosition = static_cast<std::int32_t>(static_cast<std::uint32_t>(newAthleteIndex) * (barHeight + distanceBetweenBars) + distanceBetweenBars);
    }

    const Window window(
        Window::Size{
            .width = static_cast<std::uint32_t>(windowHeight * aspectRatio),
            .height = static_cast<std::uint32_t>(windowHeight),
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

    // Load fonts.
    char* applicationBasePath = SDL_GetBasePath();

    std::string sidebarFontPath = std::string(applicationBasePath);
    sidebarFontPath += scriptEngine["FONTS"]["sidebar"];

    std::string eliminatedFontPath = std::string(applicationBasePath);
    eliminatedFontPath += scriptEngine["FONTS"]["eliminated"];

    SDL_free(applicationBasePath);
    applicationBasePath = nullptr;

    const std::int32_t fontPointSize = static_cast<std::int32_t>(static_cast<std::float_t>(barHeight) * (72.0f / 96.0f));

    TextCache textCache(TTF_OpenFont(sidebarFontPath.c_str(), fontPointSize), renderer);
    TextCache eliminatedTextCache(TTF_OpenFont(eliminatedFontPath.c_str(), fontPointSize), renderer);

    // const auto eliminatedText = eliminatedTextCache.Get("ELIMINATED", SDL_Colour(0xFFu, 0x00u, 0x00u, SDL_ALPHA_OPAQUE));

    // Get length of max score text.
    const auto maxScoreText = textCache.Get(std::to_string(maxScore));
    const auto [maxScoreTextWidth, maxScoreTextHeight] = GetTextureSize(maxScoreText);
    const std::float_t maxScoreRatio = static_cast<std::float_t>(maxScoreTextHeight) / static_cast<std::float_t>(barHeight);
    const std::int32_t newMaxScoreWidth = static_cast<std::int32_t>(static_cast<std::float_t>(maxScoreTextWidth) / maxScoreRatio);

    const std::int32_t sidebarWidth = scriptEngine["DIMENSIONS"]["sidebar_width"];

    // Get the length of the longest bar (and how much it is inset from the right window border).
    const std::int32_t maxScoreBarLength = static_cast<std::int32_t>(windowHeight * aspectRatio) - sidebarWidth - newMaxScoreWidth - 5 - 12 - 16;
    const std::float_t pixelsPerPoint = static_cast<std::float_t>(maxScoreBarLength) / static_cast<std::float_t>(maxScore);

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
                athlete.currentScore = std::lerp(static_cast<std::float_t>(athlete.originalScore), static_cast<std::float_t>(athlete.newScore), counter);
                athlete.currentPosition = std::lerp(static_cast<std::float_t>(athlete.originalPosition), static_cast<std::float_t>(athlete.newPosition), counter);
            }

            counter += 0.00005f;

            if (counter != 1.0f && counter > 1.0f) [[unlikely]]
            {
                for (auto& athlete : athletes)
                {
                    athlete.currentScore = std::lerp(static_cast<std::float_t>(athlete.originalScore), static_cast<std::float_t>(athlete.newScore), 1.0f);
                    athlete.currentPosition = std::lerp(static_cast<std::float_t>(athlete.originalPosition), static_cast<std::float_t>(athlete.newPosition), 1.0f);
                }
            }
        }

        renderer.Clear(backgroundColour);

        // Render sidebar.
        const SDL_Rect sidebar{
            .x = 0,
            .y = 0,
            .w = sidebarWidth,
            .h = static_cast<std::int32_t>(windowHeight),
        };

        renderer.DrawRectangle(sidebar, sidebarColour);

        // Render each athlete.
        for (const auto& athlete : athletes)
        {
            // Render athlete's score bar.
            const SDL_Rect currentAthleteBar{
                .x = sidebar.w,
                .y = static_cast<std::int32_t>(athlete.currentPosition),
                .w = static_cast<std::int32_t>(athlete.currentScore * pixelsPerPoint),
                .h = static_cast<std::int32_t>(barHeight),
            };

            renderer.DrawRectangle(currentAthleteBar, athlete.colour);

            // Render athlete's name.
            const auto athleteNameText = textCache.Get(athlete.name);
            const auto [nameWidth, nameHeight] = GetTextureSize(athleteNameText);

            const std::float_t textureToBarRatio = static_cast<std::float_t>(nameHeight) / static_cast<std::float_t>(barHeight);
            const std::int32_t newNameWidth = static_cast<std::int32_t>(static_cast<std::float_t>(nameWidth) / textureToBarRatio);

            const SDL_Rect currentAthleteText{
                .x = sidebarWidth - newNameWidth - 24,
                .y = static_cast<std::int32_t>(athlete.currentPosition),
                .w = newNameWidth,
                .h = static_cast<std::int32_t>(barHeight),
            };

            renderer.DrawTexture(athleteNameText, currentAthleteText, athlete.colour);

            // Render athlete's score.
            const auto athleteScoreText = textCache.Get(std::to_string(static_cast<std::uint32_t>(athlete.currentScore)));
            const auto [scoreWidth, scoreHeight] = GetTextureSize(athleteScoreText);

            const std::int32_t newScoreWidth = static_cast<std::int32_t>(static_cast<std::float_t>(scoreWidth) / textureToBarRatio);

            const SDL_Rect currentAthleteScoreText{
                .x = static_cast<std::int32_t>(athlete.currentScore * pixelsPerPoint) + 5 + 12 + sidebarWidth,
                .y = static_cast<std::int32_t>(athlete.currentPosition),
                .w = newScoreWidth,
                .h = static_cast<std::int32_t>(barHeight),
            };

            renderer.DrawTexture(athleteScoreText, currentAthleteScoreText);

            // Render ordinal number.
            const auto ordinalText = textCache.Get(GetOrdinalNumber(athlete.currentRanking + 1u));
            const auto [ordinalWidth, ordinalHeight] = GetTextureSize(ordinalText);

            const auto newOrdinalWidth = static_cast<std::int32_t>(static_cast<std::float_t>(ordinalWidth) / textureToBarRatio);

            const SDL_Rect currentOrdinalText{
                .x = 32,
                .y = athlete.originalPosition,
                .w = newOrdinalWidth,
                .h = static_cast<std::int32_t>(barHeight),
            };

            renderer.DrawTexture(ordinalText, currentOrdinalText);
        }

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

    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
