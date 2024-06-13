#ifndef PLATO_UTILITIES_STKCOMMANDGENERATOR
#define PLATO_UTILITIES_STKCOMMANDGENERATOR

#include <string>

namespace plato::utilities
{
/// @brief Simple enum class to indicate whether the STK command should generate hexs or tets. Very limited scope of
/// this enum class.
enum class STKCommandElementType
{
    Hex,
    Tet
};

/// @brief Struct to bundle the bounds for a STK generate command
struct STKCommandBounds
{
    double mX = 1;
    double mY = 1;
    double mZ = 1;
};

/// @brief Struct to bundle the number of elements in a STK generate command
struct STKCommandNumberOfElements
{
    unsigned int mX = 1u;
    unsigned int mY = 1u;
    unsigned int mZ = 1u;
};

/// @brief Struct to bundle the common operations needed to work with STK "generate" command
struct STKCommandGenerator
{
    STKCommandNumberOfElements mElements = {1u, 1u, 1u};
    STKCommandBounds mLowerBounds = {0, 0, 0};
    STKCommandBounds mUpperBounds = {1, 1, 1};
    STKCommandElementType mType = STKCommandElementType::Hex;

    /// @brief Take the struct data and return a string with the STK "generate" command
    [[nodiscard]] std::string toString(const int aPrecision = 8) const;

    /// @brief Take the struct data and determine the volume of the brick
    [[nodiscard]] double volume() const;

    /// @brief Take the struct data and determine the total number of elements
    [[nodiscard]] unsigned int numberOfElements() const;

    /// @brief Take the struct data and determine the total number of nodes
    [[nodiscard]] unsigned int numberOfNodes() const;
};

}  // namespace plato::utilities

#endif
