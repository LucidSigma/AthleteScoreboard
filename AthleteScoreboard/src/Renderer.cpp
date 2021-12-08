#include "Renderer.h"

Renderer::Renderer(const Window& window)
{
    m_handle = SDL_CreateRenderer(
        window.GetRawHandle(),
        -1,
        SDL_RENDERER_ACCELERATED
    );
}

Renderer::~Renderer() noexcept
{
    SDL_DestroyRenderer(m_handle);
}

auto Renderer::Clear(const SDL_Colour& colour) const -> void
{
    SDL_SetRenderDrawColor(m_handle, colour.r, colour.g, colour.b, colour.a);
    SDL_RenderClear(m_handle);
}

auto Renderer::DrawRectangle(const SDL_Rect& rectangle, const SDL_Colour& colour) const -> void
{
    SDL_SetRenderDrawColor(m_handle, colour.r, colour.g, colour.b, colour.a);
    SDL_RenderFillRect(m_handle, &rectangle);
}

auto Renderer::DrawTexture(SDL_Texture* const texture, const SDL_Rect& area, const std::optional<SDL_Colour>& colourMod) const -> void
{
    if (colourMod.has_value())
    {
        SDL_SetTextureColorMod(texture, colourMod.value().r, colourMod.value().g, colourMod.value().b);
    }
    else
    {
        SDL_SetTextureColorMod(texture, 0xFFu, 0xFFu, 0xFFu);
    }

    SDL_RenderCopy(m_handle, texture, nullptr, &area);
}

auto Renderer::Present() const -> void
{
    SDL_RenderPresent(m_handle);
}
