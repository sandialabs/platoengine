#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_COMMANDGENERATOR
#define PLATO_THIRDPARTYINTEGRATION_STKIO_COMMANDGENERATOR

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
enum class CommandElementType
{
    Hex,
    Tet
};

using CommandBounds = common::Coordinate;

/// @brief Struct to bundle the number of elements in a STK generate command
struct CommandNumberOfElements
{
    unsigned int mX = 1u;
    unsigned int mY = 1u;
    unsigned int mZ = 1u;
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
    CommandNumberOfElements mElements = {1u, 1u, 1u};
    CommandBounds mLowerBounds = {0, 0, 0};
    CommandBounds mUpperBounds = {1, 1, 1};
    CommandElementType mType = CommandElementType::Hex;
    NodeSetSideSetIdentifiers mNodeset{};
    NodeSetSideSetIdentifiers mSideset{};
    int mPrecision = 8;

    /// @brief Take the struct data and return a string with the STK "generate" command
    [[nodiscard]] std::string toString() const;

    /// @brief Take the struct data and determine the volume of the brick
    [[nodiscard]] double volume() const;

    /// @brief Take the struct data and determine the total number of elements
    [[nodiscard]] unsigned int numberOfElements() const;

    /// @brief Take the struct data and determine the total number of nodes
    [[nodiscard]] unsigned int numberOfNodes() const;

    /// @brief Take the struct data and return a string with the STK "nodeset" command
    [[nodiscard]] std::string nodesetString() const;

    /// @brief Take the struct data and return a string with the STK "sideset" command
    [[nodiscard]] std::string sidesetString() const;
};

namespace detail
{
/// @brief Take the struct of nodeset sideset identifiers and return lower and upper case x,y,zs for STK
[[nodiscard]] std::string xyz_boundary_string(const NodeSetSideSetIdentifiers& aNodeSetSideSetIdentifiers);

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io

#endif
