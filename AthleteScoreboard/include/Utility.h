#pragma once
#ifndef UTILITY_H
#define UTILITY_H

#include <cstdint>
#include <string>

#include <SDL2/SDL.h>

struct [[nodiscard]] TextureSize final
{
    std::uint32_t width;
    std::uint32_t height;
};

[[nodiscard]] extern auto GetTextureSize(SDL_Texture* const texture) -> TextureSize;

[[nodiscard]] extern auto GetOrdinalNumber(const std::uint32_t cardinalNumber) -> std::string;

#endif
