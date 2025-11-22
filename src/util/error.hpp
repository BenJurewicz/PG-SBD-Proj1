#ifndef ERROR_HPP
#define ERROR_HPP

#include <exception>
#include <format>
#include <source_location>
#include <type_traits>

// Wierd macro hacking to bypass the warnings that forbids the use of
// ##__VA_ARGS__ to remove the preceding coma when __VA_ARGS__ is empty -_-

// NOTE: THROW_FORMATTED is the only function you should use from this file
// Arg. 1 is the ErrorType (Must derive from std::exception)
// Arg. 2 is the string to be formated with std::format
// Args. 3... are the values that will be inserted into Arg.2
#define THROW_FORMATTED(...)                                                                 \
    THROW_FORMATTED_SELECT(__VA_ARGS__, THROW_FORMATTED_WITH_ARGS, THROW_FORMATTED_NO_ARGS)( \
        __VA_ARGS__                                                                          \
    )
#define THROW_FORMATTED_SELECT(_1, _2, _3, NAME, ...) NAME

#define THROW_FORMATTED_WITH_ARGS(ErrorType, fmt, ...) \
    _throw_formatted<ErrorType>(                       \
        fmt,                                           \
        std::source_location::current(),               \
        __VA_ARGS__                                    \
    )

#define THROW_FORMATTED_NO_ARGS(ErrorType, fmt) \
    _throw_formatted<ErrorType>(fmt, std::source_location::current())

template <typename ErrorType, typename... Args>
[[noreturn]] void _throw_formatted(
    const std::format_string<Args...> fmt, const std::source_location& sl,
    Args&&... args
) {
    static_assert(
        std::is_base_of_v<std::exception, ErrorType>,
        "Passed ErrorType does not derive from std::exception."
    );

    auto msg = std::format(fmt, args...);
    msg = std::format(
        "\n{}:{}:\nFunction: {}\n{}",
        sl.file_name(),
        sl.line(),
        sl.function_name(),
        msg
    );
    throw ErrorType(msg);
}

#endif  // !ERROR_HPP
