#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <cstdint>
#include <string_view>

#include <SDL2/SDL.h>

class [[nodiscard]] Window final
{
public:
    struct [[nodiscard]] Size final
    {
        std::uint32_t width;
        std::uint32_t height;
    };

private:
    SDL_Window* m_handle = nullptr;

public:
    Window(const Size size, const std::string_view title);
    ~Window() noexcept;

    [[nodiscard]] auto GetSize() const -> Size;

    [[nodiscard]] inline auto IsValid() const noexcept -> bool { return m_handle != nullptr; }
    [[nodiscard]] inline auto GetRawHandle() const noexcept -> SDL_Window* { return m_handle; }
};

#endif
