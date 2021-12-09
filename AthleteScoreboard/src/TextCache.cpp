#include "TextCache.h"

TextCache::TextCache(TTF_Font* const font, const Renderer& renderer)
    : m_font(font), m_renderer(&renderer)
{ }

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
            return nullptr;
        }

        m_textureLookup[text] = SDL_CreateTextureFromSurface(m_renderer->GetRawHandle(), textSurface);
    }

    const auto texture = m_textureLookup.at(text);

    return texture;
}
