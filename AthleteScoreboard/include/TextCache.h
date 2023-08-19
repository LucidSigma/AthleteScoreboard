#pragma once
#ifndef TEXT_CACHE_H
#define TEXT_CACHE_H

#include <unordered_map>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "Renderer.h"

class [[nodiscard]] TextCache final
{
private:
    TTF_Font* m_font = nullptr;
    const Renderer* m_renderer = nullptr;

    std::unordered_map<std::string, SDL_Texture*> m_textureLookup{ };

public:
    TextCache() = default;
    TextCache(TTF_Font* const font, const Renderer& renderer);
    ~TextCache() noexcept;

    auto Initialise(TTF_Font* const font, const Renderer& renderer) -> void;

    [[nodiscard]] auto Get(const std::string& text) -> SDL_Texture*;
};

#endif
