#include "Utility.h"

[[nodiscard]] auto GetTextureSize(SDL_Texture* const texture) -> TextureSize
{
    std::int32_t width = 0;
    std::int32_t height = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);

    return TextureSize{
        .width = static_cast<std::uint32_t>(width),
        .height = static_cast<std::uint32_t>(height),
    };
}

[[nodiscard]] auto GetOrdinalNumber(const std::uint32_t cardinalNumber) -> std::string
{
    const std::string numberString = std::to_string(cardinalNumber);
    const std::uint32_t remainder = cardinalNumber % 10u;

    switch (remainder)
    {
    case 1u:
        if (cardinalNumber % 100u == 11u)
        {
            return numberString + "th";
        }

        return numberString + "st";

    case 2u:
        if (cardinalNumber % 100u == 12u)
        {
            return numberString + "th";
        }

        return numberString + "nd";

    case 3u:
        if (cardinalNumber % 100u == 13u)
        {
            return numberString + "th";
        }

        return numberString + "rd";

    default:
        return numberString + "th";
    }
}
