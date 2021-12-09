#include "AthleteScoreboard.h"

#include <algorithm>
#include <cstddef>
#include <format>
#include <functional>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>

#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

#include "Utility.h"

AthleteScoreboard::AthleteScoreboard(const ScriptEngine& scriptEngine)
try
{
    LoadAthletes(scriptEngine);
    LoadDimensions(scriptEngine);
    LoadColours(scriptEngine);

    CalculateAthletePositions();
    CalculateNewAthleteScoresAndPositions();

    m_didInitialiseSuccessfully = true;
}
catch (const sol::error& error)
{
    spdlog::error("The following error occurred when loading athletes.lua: {}.", error.what());
}
catch (...)
{
    spdlog::error("An unknown error occurred when loading athletes.lua.");
}

AthleteScoreboard::~AthleteScoreboard() noexcept
{
    for (auto& font : { m_athleteFont, m_eliminatedFont })
    {
        if (font != nullptr)
        {
            TTF_CloseFont(font);
        }
    }
}

auto AthleteScoreboard::LoadTextCaches(const ScriptEngine& scriptEngine, const Renderer& renderer) -> void
{
    char* applicationBasePathPointer = SDL_GetBasePath();
    const std::string applicationBasePath = applicationBasePathPointer;
    SDL_free(applicationBasePathPointer);
    applicationBasePathPointer = nullptr;

    std::string athletesFontPath = applicationBasePath;
    athletesFontPath += scriptEngine["FONTS"]["athletes"];

    std::string eliminatedFontPath = applicationBasePath;
    eliminatedFontPath += scriptEngine["FONTS"]["eliminated"];

    const std::int32_t fontPointSize = static_cast<std::int32_t>(static_cast<std::float_t>(m_dimensions.barHeight) * (72.0f / 96.0f));

    m_athleteFont = TTF_OpenFont(athletesFontPath.c_str(), fontPointSize);

    if (m_athleteFont == nullptr)
    {
        throw std::runtime_error(std::format("Failed to load font at {}. Error: {}.", athletesFontPath, TTF_GetError()));
    }

    m_eliminatedFont = TTF_OpenFont(eliminatedFontPath.c_str(), fontPointSize);

    if (m_eliminatedFont == nullptr)
    {
        throw std::runtime_error(std::format("Failed to load font at {}. Error: {}.", eliminatedFontPath, TTF_GetError()));
    }

    m_athleteTextCache.Initialise(m_athleteFont, renderer);
    m_eliminatedTextCache.Initialise(m_eliminatedFont, renderer);

    m_eliminatedText = m_eliminatedTextCache.Get("ELIMINATED");

    CalculateMaxScoreTextWidth();
    CalculatePixelsPerPoint();

    LoadOrdinalNumberTexts();
}

auto AthleteScoreboard::Update(const std::float_t deltaTime) -> void
{
    constexpr std::float_t SecondsPerFullInterpolation = 7.5f;

    switch (m_state)
    {
    case State::Idle:
    case State::End:
        break;

    case State::UpdateScores:
        if (m_interpolation <= 1.0f)
        {
            for (auto& athlete : m_athletes)
            {
                athlete.currentScore = std::lerp(static_cast<std::float_t>(athlete.originalScore), static_cast<std::float_t>(athlete.newScore), m_interpolation);
            }

            m_interpolation += (1.0f / SecondsPerFullInterpolation) * deltaTime;

            if (m_interpolation != 1.0f && m_interpolation > 1.0f) [[unlikely]]
            {
                for (auto& athlete : m_athletes)
                {
                    athlete.currentScore = std::lerp(static_cast<std::float_t>(athlete.originalScore), static_cast<std::float_t>(athlete.newScore), 1.0f);
                }
            }
        }
        else if (!m_readyToChangeState)
        {
            m_readyToChangeState = true;
        }

        break;

    case State::UpdatePositions:
        if (m_interpolation <= 1.0f)
        {
            for (auto& athlete : m_athletes)
            {
                athlete.currentPosition = std::lerp(static_cast<std::float_t>(athlete.originalPosition), static_cast<std::float_t>(athlete.newPosition), m_interpolation);
            }

            m_interpolation += (1.0f / SecondsPerFullInterpolation) * deltaTime;

            if (m_interpolation != 1.0f && m_interpolation > 1.0f) [[unlikely]]
            {
                for (auto& athlete : m_athletes)
                {
                    athlete.currentPosition = std::lerp(static_cast<std::float_t>(athlete.originalPosition), static_cast<std::float_t>(athlete.newPosition), 1.0f);
                }
            }
        }
        else if (!m_readyToChangeState)
        {
            m_readyToChangeState = true;
        }

        break;

    case State::DisplayEliminatedText:
        if (m_interpolation == 0.0f)
        {
            std::ranges::sort(m_athletes, std::greater());

            for (auto& athlete : std::ranges::reverse_view(m_athletes))
            {
                if (!athlete.isEliminated)
                {
                    athlete.isEliminated = true;
                    m_newlyEliminatedAthleteName = athlete.name;

                    break;
                }
            }
        }

        m_interpolation += (1.0f / SecondsPerFullInterpolation) * deltaTime;

        if (m_interpolation >= 1.0f)
        {
            m_interpolation = 1.0f;
            m_newlyEliminatedAthleteName = "";

            m_state = State::End;
        }

        break;
    }
}

auto AthleteScoreboard::Render(const Renderer& renderer) -> void
{
    renderer.Clear(m_colours.background);

    RenderSidebar(renderer);
    RenderOrdinalNumbers(renderer);

    for (const auto& athlete : m_athletes)
    {
        RenderAthleteScoreBar(renderer, athlete);
        RenderAthleteName(renderer, athlete);
        RenderAthleteScoreBarText(renderer, athlete);
    }
}

auto AthleteScoreboard::HandleKeyPress(const SDL_Scancode scancode) -> void
{
    if (scancode == SDL_SCANCODE_RETURN && m_readyToChangeState)
    {
        switch (m_state)
        {
        case State::Idle:
            m_state = State::UpdateScores;
            m_readyToChangeState = false;
            m_interpolation = 0.0f;

            break;

        case State::UpdateScores:
            m_state = State::UpdatePositions;
            m_readyToChangeState = false;
            m_interpolation = 0.0f;

            break;

        case State::UpdatePositions:
            m_state = State::DisplayEliminatedText;
            m_readyToChangeState = false;
            m_interpolation = 0.0f;

            break;
        }
    }
}


auto AthleteScoreboard::LoadAthletes(const ScriptEngine& scriptEngine) -> void
{
    for (const auto& [athleteName, athleteData] : scriptEngine.Get<sol::table>("ATHLETES"))
    {
        const bool isAthleteEliminated = athleteData.as<sol::table>()["is_eliminated"];

        m_athletes.push_back(
            Athlete{
                .name = athleteName.as<std::string>(),
                .colour = athleteData.as<sol::table>()["colour"],
                .pointsToAdd = isAthleteEliminated ? 0 : athleteData.as<sol::table>()["points_to_add"],
                .isEliminated = isAthleteEliminated,
                .originalScore = athleteData.as<sol::table>()["current_score"],
            }
        );
    }

    std::ranges::sort(m_athletes, std::greater());
}

auto AthleteScoreboard::LoadDimensions(const ScriptEngine& scriptEngine) -> void
{
    m_dimensions.aspectRatio = scriptEngine["DIMENSIONS"]["aspect_ratio"];
    m_dimensions.barHeight = scriptEngine["DIMENSIONS"]["bar_height"];
    m_dimensions.distanceBetweenBars = scriptEngine["DIMENSIONS"]["distance_between_bars"];
    m_dimensions.minScoreBarLength = scriptEngine["DIMENSIONS"]["min_score_bar_length"];
    m_dimensions.sidebarWidth = scriptEngine["DIMENSIONS"]["sidebar_width"];
    m_dimensions.distanceBetweenBarAndScoreText = scriptEngine["DIMENSIONS"]["distance_between_bar_and_score_text"];
    m_dimensions.distanceBetweenScoreTextAndWindowRight = scriptEngine["DIMENSIONS"]["distance_between_score_text_and_window_right"];
    m_dimensions.distanceBetweenScoreTextAndEliminatedText = scriptEngine["DIMENSIONS"]["distance_between_score_text_and_eliminated_text"];
    m_dimensions.distanceBetweenNameAndSidebar = scriptEngine["DIMENSIONS"]["distance_between_name_and_sidebar"];
    m_dimensions.distanceBetweenOrdinalNumbersAndWindowLeft = scriptEngine["DIMENSIONS"]["distance_between_ordinal_numbers_and_window_left"];

    m_windowHeight = static_cast<std::float_t>(
        (static_cast<std::uint32_t>(m_athletes.size()) * (m_dimensions.barHeight + m_dimensions.distanceBetweenBars)) + m_dimensions.distanceBetweenBars
    );
}

auto AthleteScoreboard::LoadColours(const ScriptEngine& scriptEngine) -> void
{
    m_colours.background = scriptEngine["COLOURS"]["background"];
    m_colours.sidebar = scriptEngine["COLOURS"]["sidebar"];
    m_colours.ordinalText = scriptEngine["COLOURS"]["ordinal_text"];
    m_colours.scoreText = scriptEngine["COLOURS"]["score_text"];
    m_colours.eliminatedText = scriptEngine["COLOURS"]["eliminated_text"];
}

auto AthleteScoreboard::CalculateAthletePositions() -> void
{
    std::int32_t yOffset = static_cast<std::int32_t>(m_dimensions.distanceBetweenBars);

    for (std::size_t i = 0u; i < m_athletes.size(); ++i)
    {
        m_athletes[i].originalPosition = static_cast<std::int32_t>(yOffset);
        m_athletes[i].currentPosition = static_cast<std::float_t>(m_athletes[i].originalPosition);
        m_athletes[i].currentScore = static_cast<std::float_t>(m_athletes[i].originalScore);

        yOffset += static_cast<std::int32_t>(m_dimensions.barHeight + m_dimensions.distanceBetweenBars);
    }
}

auto AthleteScoreboard::CalculateNewAthleteScoresAndPositions() -> void
{
    std::vector<Athlete> newAthletes = m_athletes;
    m_maxScore = 0u;

    for (auto& athlete : newAthletes)
    {
        athlete.currentScore += static_cast<std::float_t>(athlete.pointsToAdd);
        m_maxScore = std::max(m_maxScore, static_cast<std::uint32_t>(athlete.currentScore));
    }

    std::ranges::sort(newAthletes, std::greater());

    for (auto& athlete : m_athletes)
    {
        const auto newAthleteLocation = std::ranges::find_if(
            newAthletes,
            [&targetName = athlete.name](const Athlete& athlete) -> bool { return athlete.name == targetName; }
        );

        const std::size_t newAthleteIndex = newAthleteLocation - std::cbegin(newAthletes);

        athlete.pointsToAdd = 0;

        athlete.newScore = static_cast<std::uint32_t>(newAthleteLocation->currentScore);
        athlete.newPosition = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(newAthleteIndex) * (m_dimensions.barHeight + m_dimensions.distanceBetweenBars) + m_dimensions.distanceBetweenBars
        );
    }
}

auto AthleteScoreboard::CalculateMaxScoreTextWidth() -> void
{
    const auto maxScoreText = m_athleteTextCache.Get(std::to_string(m_maxScore));
    const auto [maxScoreTextWidth, maxScoreTextHeight] = GetTextureSize(maxScoreText);
    const std::float_t maxScoreRatio = static_cast<std::float_t>(maxScoreTextHeight) / static_cast<std::float_t>(m_dimensions.barHeight);
    m_maxScoreTextWidth = static_cast<std::int32_t>(static_cast<std::float_t>(maxScoreTextWidth) / maxScoreRatio);
}

auto AthleteScoreboard::CalculatePixelsPerPoint() -> void
{
    const std::int32_t maxScoreBarLength =
        static_cast<std::int32_t>(m_windowHeight * m_dimensions.aspectRatio) -
        m_dimensions.sidebarWidth -
        m_maxScoreTextWidth -
        m_dimensions.minScoreBarLength -
        m_dimensions.distanceBetweenBarAndScoreText -
        m_dimensions.distanceBetweenScoreTextAndWindowRight;

    m_pixelsPerPoint = static_cast<std::float_t>(maxScoreBarLength) / static_cast<std::float_t>(m_maxScore);
}

auto AthleteScoreboard::LoadOrdinalNumberTexts() -> void
{
    for (std::uint32_t i = 1u; i <= static_cast<std::uint32_t>(m_athletes.size()); ++i)
    {
        m_ordinalNumberTexts[i] = m_athleteTextCache.Get(GetOrdinalNumber(i));
    }
}

auto AthleteScoreboard::RenderSidebar(const Renderer& renderer) -> void
{
    const SDL_Rect sidebarArea{
        .x = 0,
        .y = 0,
        .w = m_dimensions.sidebarWidth,
        .h = static_cast<std::int32_t>(m_windowHeight),
    };

    renderer.DrawRectangle(sidebarArea, m_colours.sidebar);
}

auto AthleteScoreboard::RenderOrdinalNumbers(const Renderer& renderer) -> void
{
    std::int32_t yOffset = static_cast<std::int32_t>(m_dimensions.distanceBetweenBars);

    for (std::uint32_t i = 1u; i <= static_cast<std::uint32_t>(m_athletes.size()); ++i)
    {
        const auto ordinalText = m_ordinalNumberTexts[i];
        const auto [ordinalTextWidth, ordinalTextHeight] = GetTextureSize(ordinalText);
        const std::float_t textureToBarRatio = static_cast<std::float_t>(ordinalTextHeight) / static_cast<std::float_t>(m_dimensions.barHeight);

        const auto proportionalOrdinalTextWidth = static_cast<std::int32_t>(static_cast<std::float_t>(ordinalTextWidth) / textureToBarRatio);

        const SDL_Rect currentOrdinalText{
            .x = m_dimensions.distanceBetweenOrdinalNumbersAndWindowLeft,
            .y = yOffset,
            .w = proportionalOrdinalTextWidth,
            .h = static_cast<std::int32_t>(m_dimensions.barHeight),
        };

        renderer.DrawTexture(ordinalText, currentOrdinalText, m_colours.ordinalText);

        yOffset += static_cast<std::int32_t>(m_dimensions.barHeight + m_dimensions.distanceBetweenBars);
    }
}

auto AthleteScoreboard::RenderAthleteScoreBar(const Renderer& renderer, const Athlete& athlete) -> void
{
    const SDL_Rect athleteScoreBarArea{
            .x = m_dimensions.sidebarWidth,
            .y = static_cast<std::int32_t>(athlete.currentPosition),
            .w = static_cast<std::int32_t>(athlete.currentScore * m_pixelsPerPoint) + m_dimensions.minScoreBarLength,
            .h = static_cast<std::int32_t>(m_dimensions.barHeight),
    };

    renderer.DrawRectangle(athleteScoreBarArea, athlete.colour);
}

auto AthleteScoreboard::RenderAthleteName(const Renderer& renderer, const Athlete& athlete) -> void
{
    const auto athleteNameText = m_athleteTextCache.Get(athlete.name);
    const auto [nameWidth, nameHeight] = GetTextureSize(athleteNameText);

    const std::float_t textureToBarRatio = static_cast<std::float_t>(nameHeight) / static_cast<std::float_t>(m_dimensions.barHeight);
    const std::int32_t newNameWidth = static_cast<std::int32_t>(static_cast<std::float_t>(nameWidth) / textureToBarRatio);

    const SDL_Rect athleteNameArea{
        .x = m_dimensions.sidebarWidth - newNameWidth - m_dimensions.distanceBetweenNameAndSidebar,
        .y = static_cast<std::int32_t>(athlete.currentPosition),
        .w = newNameWidth,
        .h = static_cast<std::int32_t>(m_dimensions.barHeight),
    };

    renderer.DrawTexture(athleteNameText, athleteNameArea, athlete.colour);
}

auto AthleteScoreboard::RenderAthleteScoreBarText(const Renderer& renderer, const Athlete& athlete) -> void
{
    const auto athleteScoreText = m_athleteTextCache.Get(std::to_string(static_cast<std::uint32_t>(athlete.currentScore)));
    const auto [scoreWidth, scoreHeight] = GetTextureSize(athleteScoreText);

    const std::float_t scoreTextureToBarRatio = static_cast<std::float_t>(scoreHeight) / static_cast<std::float_t>(m_dimensions.barHeight);
    const std::int32_t newScoreWidth = static_cast<std::int32_t>(static_cast<std::float_t>(scoreWidth) / scoreTextureToBarRatio);

    const SDL_Rect athleteScoreTextArea{
        .x = static_cast<std::int32_t>(athlete.currentScore * m_pixelsPerPoint) +
            m_dimensions.minScoreBarLength +
            m_dimensions.distanceBetweenBarAndScoreText +
            m_dimensions.sidebarWidth,
        .y = static_cast<std::int32_t>(athlete.currentPosition),
        .w = newScoreWidth,
        .h = static_cast<std::int32_t>(m_dimensions.barHeight),
    };

    renderer.DrawTexture(athleteScoreText, athleteScoreTextArea, m_colours.scoreText);

    if (athlete.isEliminated)
    {
        const auto [eliminatedTextWidth, eliminatedTextHeight] = GetTextureSize(m_eliminatedText);
        const std::float_t eliminatedTextureToBarRatio = static_cast<std::float_t>(eliminatedTextHeight) / static_cast<std::float_t>(m_dimensions.barHeight);
        const std::int32_t newEliminatedTextWidth = static_cast<std::int32_t>(static_cast<std::float_t>(eliminatedTextWidth) / eliminatedTextureToBarRatio);

        const SDL_Rect eliminatedTextArea{
            .x = static_cast<std::int32_t>(athlete.currentScore * m_pixelsPerPoint) +
                m_dimensions.minScoreBarLength +
                m_dimensions.distanceBetweenBarAndScoreText +
                m_dimensions.sidebarWidth +
                newScoreWidth +
                m_dimensions.distanceBetweenScoreTextAndEliminatedText,
            .y = static_cast<std::int32_t>(athlete.currentPosition),
            .w = newEliminatedTextWidth,
            .h = static_cast<std::int32_t>(m_dimensions.barHeight),
        };

        SDL_Colour eliminatedTextColour = m_colours.eliminatedText;

        if (m_newlyEliminatedAthleteName == athlete.name)
        {
            eliminatedTextColour.a = static_cast<std::uint8_t>(m_interpolation * 255.0f);
        }

        renderer.DrawTexture(m_eliminatedText, eliminatedTextArea, eliminatedTextColour);
    }
}
