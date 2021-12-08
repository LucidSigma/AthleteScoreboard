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
        if (numberString.length() >= 2u)
        {
            const std::string lastTwoCharacters = numberString.substr(numberString.length() - 2u);

            if (lastTwoCharacters == "11")
            {
                return numberString + "th";
            }
        }

        return numberString + "st";

    case 2u:
        if (numberString.length() >= 2u)
        {
            const std::string lastTwoCharacters = numberString.substr(numberString.length() - 2u);

            if (lastTwoCharacters == "12")
            {
                return numberString + "th";
            }
        }

        return numberString + "nd";

    case 3u:
        if (numberString.length() >= 2u)
        {
            const std::string lastTwoCharacters = numberString.substr(numberString.length() - 2u);

            if (lastTwoCharacters == "13")
            {
                return numberString + "th";
            }
        }

        return numberString + "rd";

    default:
        return numberString + "th";
    }
}
