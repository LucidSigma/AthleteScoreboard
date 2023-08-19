#include "TextCache.h"

#include <spdlog/spdlog.h>

TextCache::TextCache(TTF_Font* const font, const Renderer& renderer)
    : m_font(font), m_renderer(&renderer)
{ }

TextCache::~TextCache() noexcept
{
    for (const auto& [name, texture] : m_textureLookup)
    {
        SDL_DestroyTexture(texture);
    }

    m_textureLookup.clear();
}

auto TextCache::Initialise(TTF_Font* const font, const Renderer& renderer) -> void
{
    m_font = font;
    m_renderer = &renderer;
}

[[nodiscard]] auto TextCache::Get(const std::string& text) -> SDL_Texture*
{
    if (!m_textureLookup.contains(text))
    {
        SDL_Surface* textSurface = TTF_RenderText_Blended(m_font, text.c_str(), SDL_Colour{ 0xFFu, 0xFFu, 0xFFu, SDL_ALPHA_OPAQUE });

        if (textSurface == nullptr)
        {
            spdlog::error("Failed to create pixel surface for text \"{}\".", text);

            return nullptr;
        }

        if (const auto textTexture = SDL_CreateTextureFromSurface(m_renderer->GetRawHandle(), textSurface);
            textTexture != nullptr)
        {
            m_textureLookup.try_emplace(text, textTexture);
        }
        else
        {
            spdlog::error("Failed to create texture for text \"{}\".", text);

            return nullptr;
        }

        SDL_FreeSurface(textSurface);
        textSurface = nullptr;
    }

    const auto texture = m_textureLookup.at(text);

    return texture;
}
