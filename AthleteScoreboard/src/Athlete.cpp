#include "Athlete.h"

[[nodiscard]] auto operator <=>(const Athlete& lhs, const Athlete& rhs) noexcept -> std::partial_ordering
{
    if (lhs.currentScore == rhs.currentScore)
    {
        if (lhs.originalScore == rhs.originalScore)
        {
            return lhs.name <=> rhs.name;
        }

        return lhs.originalScore <=> rhs.originalScore;
    }

    return lhs.currentScore <=> rhs.currentScore;
}
