#pragma once
#ifndef ATHLETE_SCOREBOARD_H
#define ATHLETE_SCOREBOARD_H

#include <cmath>
#include <cstdint>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Athlete.h"
#include "Renderer.h"
#include "ScriptEngine.h"
#include "TextCache.h"
#include "Window.h"

class [[nodiscard]] AthleteScoreboard final
{
private:
    enum class [[nodiscard]] State
    {
        Idle,
        UpdateScores,
        UpdatePositions,
        DisplayEliminatedText,
        End,
    };

    struct [[nodiscard]] Dimensions final
    {
        std::float_t aspectRatio = 0.0f;

        std::uint32_t barHeight = 0u;
        std::uint32_t distanceBetweenBars = 0u;

        std::int32_t minimumScoreBarLength = 0;
        std::int32_t sidebarWidth = 0;
        std::int32_t distanceBetweenBarAndScoreText = 0;
        std::int32_t distanceBetweenScoreTextAndWindowRight = 0;
        std::int32_t distanceBetweenScoreTextAndEliminatedText = 0;
        std::int32_t distanceBetweenNameAndSidebar = 0;

        std::int32_t distanceBetweenOrdinalNumbersAndWindowLeft = 0;
        std::int32_t distanceBetweenEliminatedTextAndWindowRight = 0;
    };

    struct [[nodiscard]] Colours final
    {
        SDL_Colour background{ 0x00u, 0x00u, 0x00u, SDL_ALPHA_OPAQUE };
        SDL_Colour sidebar{ 0x00u, 0x00u, 0x00u, SDL_ALPHA_OPAQUE };
        SDL_Colour ordinalText{ 0x00u, 0x00u, 0x00u, SDL_ALPHA_OPAQUE };
        SDL_Colour scoreText{ 0x00u, 0x00u, 0x00u, SDL_ALPHA_OPAQUE };
        SDL_Colour eliminatedText{ 0x00u, 0x00u, 0x00u, SDL_ALPHA_OPAQUE };
    };

    State m_state = State::Idle;
    bool m_readyToChangeState = true;

    Dimensions m_dimensions{ };
    std::float_t m_windowHeight = 0.0f;

    Colours m_colours{ };

    std::vector<Athlete> m_athletes{ };
    std::uint32_t m_maximumScore = 0u;
    std::int32_t m_maximumScoreTextWidth = 0;
    std::float_t m_pixelsPerPoint = 0.0f;

    std::function<auto(std::float_t) -> std::float_t> m_easingFunction;

    TextCache m_athleteTextCache;
    TTF_Font* m_athleteFont = nullptr;

    TextCache m_eliminatedTextCache;
    TTF_Font* m_eliminatedFont = nullptr;

    std::unordered_map<std::uint32_t, SDL_Texture*> m_ordinalNumberTexts{ };
    SDL_Texture* m_eliminatedText = nullptr;
    std::string_view m_newlyEliminatedAthleteName = "";

    std::float_t m_interpolation = 0.0f;

    bool m_didInitialiseSuccessfully = false;

public:
    explicit AthleteScoreboard(const ScriptEngine& scriptEngine);
    ~AthleteScoreboard() noexcept;

    auto LoadTextCaches(const ScriptEngine& scriptEngine, const Renderer& renderer) -> void;

    auto Update(const std::float_t deltaTime) -> void;
    auto Render(const Renderer& renderer) -> void;
    auto HandleKeyPress(const SDL_Scancode scancode) -> void;

    [[nodiscard]] inline auto DidInitialiseSuccessfully() const noexcept -> bool { return m_didInitialiseSuccessfully; }

    [[nodiscard]] inline auto GetAspectRatio() const noexcept -> std::float_t { return m_dimensions.aspectRatio; }
    [[nodiscard]] inline auto GetWindowHeight() const noexcept -> std::float_t { return m_windowHeight; }

private:
    auto LoadAthletes(const ScriptEngine& scriptEngine) -> void;
    auto LoadDimensions(const ScriptEngine& scriptEngine) -> void;
    auto LoadColours(const ScriptEngine& scriptEngine) -> void;

    auto CalculateAthletePositions() -> void;
    auto CalculateNewAthleteScoresAndPositions() -> void;

    auto CalculateMaximumScoreTextWidth() -> void;
    auto CalculatePixelsPerPoint() -> void;
    auto LoadOrdinalNumberTexts() -> void;

    auto RenderSidebar(const Renderer& renderer) const -> void;
    auto RenderOrdinalNumbers(const Renderer& renderer) -> void;
    auto RenderAthleteScoreBar(const Renderer& renderer, const Athlete& athlete) -> void;
    auto RenderAthleteName(const Renderer& renderer, const Athlete& athlete) -> void;
    auto RenderAthleteScoreBarText(const Renderer& renderer, const Athlete& athlete) -> void;
};

#endif
