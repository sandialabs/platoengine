#ifndef PLATO_UTILITIES_FIXEDWIDTHFLOATINGPOINTOUTPUT
#define PLATO_UTILITIES_FIXEDWIDTHFLOATINGPOINTOUTPUT

#include <iomanip>
#include <sstream>
#include <type_traits>

namespace plato::utilities
{

///@brief This is a utility struct that facilitates specialized formatting of numbers in tables.
///
/// @code{.cpp}
/// const auto tPi = FixedWidthFloatingPointOutput<double, 3U, 10U>{3.1415926};
/// const auto tE = FixedWidthFloatingPointOutput<double, 3U, 10U>{2.7182818};
/// std::cout << tPi << tE << std::endl;
/// @endcode
/// Produces the output string "      3.14      2.72".
template <typename T, std::size_t Precision, std::size_t FieldWidth>
struct FixedWidthFloatingPointOutput
{
    static_assert(std::is_floating_point_v<T>, "Must be instantiated with a floating point type.");
    T mValue;
};

/// @brief Converts @a aValue to a string using the field width and precision given by FixedWidthFloatingPointOutput.
template <typename T, std::size_t Precision, std::size_t FieldWidth>
[[nodiscard]] auto to_string(const FixedWidthFloatingPointOutput<T, Precision, FieldWidth>& aValue) -> std::string;

template <typename Stream, typename T, std::size_t Precision, std::size_t FieldWidth>
auto operator<<(Stream& aStream, FixedWidthFloatingPointOutput<T, Precision, FieldWidth> aFloatingValue) -> Stream&
{
    aStream << std::setprecision(Precision) << std::setw(FieldWidth) << aFloatingValue.mValue;
    return aStream;
}

template <typename T, std::size_t Precision, std::size_t FieldWidth>
[[nodiscard]] auto to_string(const FixedWidthFloatingPointOutput<T, Precision, FieldWidth>& aValue) -> std::string
{
    auto tStream = std::stringstream{};
    tStream << aValue;
    return tStream.str();
}

}  // namespace plato::utilities

#endif
