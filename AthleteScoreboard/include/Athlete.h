#pragma once
#ifndef ATHLETE_H
#define ATHLETE_H

#include <cmath>
#include <cstdint>
#include <string>

#include <SDL2/SDL.h>

struct [[nodiscard]] Athlete final
{
    std::string name;
    SDL_Colour colour;
    std::uint32_t ranking; // TODO: Remove.

    std::int32_t pointsToAdd;
    bool isEliminated;

    std::uint32_t originalScore;
    std::float_t currentScore;
    std::uint32_t newScore;


    std::int32_t originalPosition;
    std::float_t currentPosition;
    std::int32_t newPosition;
};

[[nodiscard]] extern auto operator >(const Athlete& lhs, const Athlete& rhs) noexcept -> bool;

#endif
