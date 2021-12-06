#pragma once
#ifndef SCRIPT_ENGINE_H
#define SCRIPT_ENGINE_H

#include <string_view>

#include <sol/sol.hpp>

class ScriptEngine final
{
private:
    sol::state m_luaState{ };

    bool m_isValid = false;

public:
    ScriptEngine();

    [[nodiscard]] inline auto IsValid() const noexcept -> bool { return m_isValid; }

    template <typename T>
    [[nodiscard]] inline auto Get(const std::string_view variableName) const -> T
    {
        return m_luaState.get<T>(variableName);
    }

    [[nodiscard]] inline auto operator [](const std::string_view variableName) -> decltype(auto) { return m_luaState[variableName]; }
};

#endif
