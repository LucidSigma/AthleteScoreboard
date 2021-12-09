#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include <optional>

#include <SDL2/SDL.h>

#include "Window.h"

class [[nodiscard]] Renderer final
{
private:
    SDL_Renderer* m_handle = nullptr;

public:
    explicit Renderer(const Window& window);
    ~Renderer() noexcept;

    auto Clear(const SDL_Colour& colour) const -> void;

    auto DrawRectangle(const SDL_Rect& rectangle, const SDL_Colour& colour) const -> void;
    auto DrawTexture(SDL_Texture* const texture, const SDL_Rect& area, const std::optional<SDL_Colour>& colourMod = std::nullopt) const -> void;

    auto Present() const -> void;

    [[nodiscard]] inline auto IsValid() const noexcept -> bool { return m_handle != nullptr; }
    [[nodiscard]] inline auto GetRawHandle() const noexcept -> SDL_Renderer* { return m_handle; }
};

#endif
