#include "Window.h"

Window::Window(const Size size, const std::string_view title)
{
    m_handle = SDL_CreateWindow(
        title.data(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<std::int32_t>(size.width),
        static_cast<std::int32_t>(size.height),
        SDL_WINDOW_SHOWN
    );
}

Window::~Window() noexcept
{
    SDL_DestroyWindow(m_handle);
}

[[nodiscard]] auto Window::GetSize() const -> Size
{
    std::int32_t width = 0;
    std::int32_t height = 0;
    SDL_GetWindowSize(m_handle, &width, &height);

    return Size{
        .width = static_cast<std::uint32_t>(width),
        .height = static_cast<std::uint32_t>(height),
    };
}
