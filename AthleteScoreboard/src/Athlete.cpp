#include "Athlete.h"

[[nodiscard]] auto operator <=>(const Athlete& lhs, const Athlete& rhs) noexcept -> std::partial_ordering
{
    if (lhs.currentScore == rhs.currentScore)
    {
        return lhs.name <=> rhs.name;
    }

    return lhs.currentScore <=> rhs.currentScore;
}
