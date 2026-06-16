#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_COMMANDGENERATOR
#define PLATO_THIRDPARTYINTEGRATION_STKIO_COMMANDGENERATOR

#include <cstdint>
#include <string>

#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::stk_io
{
using UseLowerX = utilities::NamedType<bool, struct UseLowerXTag>;
using UseUpperX = utilities::NamedType<bool, struct UseUpperXTag>;
using UseLowerY = utilities::NamedType<bool, struct UseLowerYTag>;
using UseUpperY = utilities::NamedType<bool, struct UseUpperYTag>;
using UseLowerZ = utilities::NamedType<bool, struct UseLowerZTag>;
using UseUpperZ = utilities::NamedType<bool, struct UseUpperZTag>;

/// @brief Simple enum class to indicate whether the STK command should generate hexs or tets. Very limited scope of
/// this enum class.
enum class CommandElementType : std::uint8_t
{
    kHex,
    kTet
};

using CommandBounds = common::Coordinate;

/// @brief Struct to bundle the number of elements in a STK generate command
struct CommandNumberOfElements
{
    unsigned int mX = 1U;
    unsigned int mY = 1U;
    unsigned int mZ = 1U;
};

struct NodeSetSideSetIdentifiers
{
    UseLowerX mLowerX{false};
    UseUpperX mUpperX{false};
    UseLowerY mLowerY{false};
    UseUpperY mUpperY{false};
    UseLowerZ mLowerZ{false};
    UseUpperZ mUpperZ{false};
};

/// @brief Struct to bundle the common operations needed to work with STK "generate" command
struct CommandGenerator
{
    constexpr static auto mDefaultPrecision = 8;

    CommandNumberOfElements mElements = {1U, 1U, 1U};
    CommandBounds mLowerBounds = {0, 0, 0};
    CommandBounds mUpperBounds = {1, 1, 1};
    CommandElementType mType = CommandElementType::kHex;
    NodeSetSideSetIdentifiers mNodeset{};
    NodeSetSideSetIdentifiers mSideset{};
    int mPrecision = mDefaultPrecision;

    /// @brief Take the struct data and return a string with the STK "generate" command
    [[nodiscard]] auto toString() const -> std::string;

    /// @brief Take the struct data and determine the volume of the brick
    [[nodiscard]] auto volume() const -> double;

    /// @brief Take the struct data and determine the total number of elements
    [[nodiscard]] auto numberOfElements() const -> unsigned int;

    /// @brief Take the struct data and determine the total number of nodes
    [[nodiscard]] auto numberOfNodes() const -> unsigned int;

    /// @brief Take the struct data and return a string with the STK "nodeset" command
    [[nodiscard]] auto nodesetString() const -> std::string;

    /// @brief Take the struct data and return a string with the STK "sideset" command
    [[nodiscard]] auto sidesetString() const -> std::string;
};

namespace detail
{
/// @brief Take the struct of nodeset sideset identifiers and return lower and upper case x,y,zs for STK
[[nodiscard]] auto xyz_boundary_string(const NodeSetSideSetIdentifiers& aNodeSetSideSetIdentifiers) -> std::string;

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io

#endif
