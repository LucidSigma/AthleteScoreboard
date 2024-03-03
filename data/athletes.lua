COLOURS = {
    sidebar = create_colour(56, 56, 56),
    background = create_colour(8, 16, 16),
    ordinal_text = create_colour(255, 255, 255),
    score_text = create_colour(255, 255, 255),
    eliminated_text = create_colour(255, 0, 0),
}

DIMENSIONS = {
    aspect_ratio = 16.0 / 9.0,
    bar_height = 37,
    distance_between_bars = 12,
    minimum_score_bar_length = 8,
    sidebar_width = 288,
    distance_between_bar_and_score_text = 12,
    distance_between_score_text_and_window_right = 12,
    distance_between_score_text_and_eliminated_text = 12,
    distance_between_name_and_sidebar = 24,
    distance_between_ordinal_numbers_and_window_left = 12,
    distance_between_eliminated_text_and_window_right = 24,
}

FONTS = {
    athletes = "data/fonts/Manrope-Bold.ttf",
    eliminated = "data/fonts/Manrope-ExtraBold.ttf",
}

EASINGS = {
    ordering = function(x)
        return 1.0 - ((1.0 - x) ^ 2.0)
    end,
}

ATHLETES = {
    Azure = {
        colour = create_colour(0, 127, 255),

        current_score = 1,
        points_to_add = 0,

        is_eliminated = true,
    },
    Black = {
        colour = create_colour(0, 0, 0),

        current_score = 2,
        points_to_add = 125,

        is_eliminated = false,
    },
    Blue = {
        colour = create_colour(0, 0, 255),

        current_score = 3,
        points_to_add = 113,

        is_eliminated = false,
    },
    Brown = {
        colour = create_colour(99, 51, 28),

        current_score = 4,
        points_to_add = 100,

        is_eliminated = false,
    },
    Cyan = {
        colour = create_colour(0, 255, 255),

        current_score = 5,
        points_to_add = 88,

        is_eliminated = false,
    },
    Forest = {
        colour = create_colour(0, 100, 0),

        current_score = 6,
        points_to_add = 75,

        is_eliminated = false,
    },
    Gold = {
        colour = create_colour(255, 165, 0),

        current_score = 7,
        points_to_add = 69,

        is_eliminated = false,
    },
    Green = {
        colour = create_colour(45, 185, 45),

        current_score = 8,
        points_to_add = 63,

        is_eliminated = false,
    },
    Grey = {
        colour = create_colour(128, 128, 128),

        current_score = 10,
        points_to_add = 56,

        is_eliminated = false,
    },
    Indigo = {
        colour = create_colour(75, 0, 130),

        current_score = 12,
        points_to_add = 50,

        is_eliminated = false,
    },
    Lavender = {
        colour = create_colour(170, 170, 238),

        current_score = 14,
        points_to_add = 44,

        is_eliminated = false,
    },
    Lime = {
        colour = create_colour(0, 255, 0),

        current_score = 16,
        points_to_add = 38,

        is_eliminated = false,
    },
    Magenta = {
        colour = create_colour(255, 0, 255),

        current_score = 18,
        points_to_add = 31,

        is_eliminated = false,
    },
    Maroon = {
        colour = create_colour(128, 0, 0),

        current_score = 20,
        points_to_add = 28,

        is_eliminated = false,
    },
    Navy = {
        colour = create_colour(0, 0, 128),

        current_score = 22,
        points_to_add = 25,

        is_eliminated = false,
    },
    Olive = {
        colour = create_colour(128, 128, 0),

        current_score = 25,
        points_to_add = 23,

        is_eliminated = false,
    },
    Orange = {
        colour = create_colour(255, 109, 0),

        current_score = 30,
        points_to_add = 20,

        is_eliminated = false,
    },
    Peach = {
        colour = create_colour(255, 203, 164),

        current_score = 35,
        points_to_add = 18,

        is_eliminated = false,
    },
    Pink = {
        colour = create_colour(255, 188, 244),

        current_score = 40,
        points_to_add = 15,

        is_eliminated = false,
    },
    Purple = {
        colour = create_colour(160, 32, 240),

        current_score = 45,
        points_to_add = 13,

        is_eliminated = false,
    },
    Red = {
        colour = create_colour(255, 0, 0),

        current_score = 50,
        points_to_add = 10,

        is_eliminated = false,
    },
    Silver = {
        colour = create_colour(192, 192, 192),

        current_score = 55,
        points_to_add = 9,

        is_eliminated = false,
    },
    Tan = {
        colour = create_colour(210, 180, 140),

        current_score = 60,
        points_to_add = 8,

        is_eliminated = false,
    },
    Teal = {
        colour = create_colour(0, 128, 128),

        current_score = 70,
        points_to_add = 6,

        is_eliminated = false,
    },
    Turquoise = {
        colour = create_colour(0, 250, 154),

        current_score = 80,
        points_to_add = 5,

        is_eliminated = false,
    },
    White = {
        colour = create_colour(255, 255, 255),

        current_score = 90,
        points_to_add = 4,

        is_eliminated = false,
    },
    Yellow = {
        colour = create_colour(255, 255, 0),

        current_score = 100,
        points_to_add = 3,

        is_eliminated = false,
    },
}
