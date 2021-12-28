# Athlete Scoreboard
A scoreboard for point-based event series.

## Usage.
Edit the fields in the [athletes.lua](data/athletes.lua) file in the data directory to match your series. Run the executable in the same directory as the data directory. The executable will require certain SDL-related DLLs if on Windows.

Press enter/return to cycle through each stage (adding points, shifting athlete's positions, displaying elimination).

## Building
The project can be build with [Premake](https://premake.github.io/).

A prebuilt Windows ZIP containing the executable, data, and DLLs is provided in the releases.
