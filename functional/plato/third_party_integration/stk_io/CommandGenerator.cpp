#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

#include <iomanip>
#include <sstream>
#include <string>

namespace plato::third_party_integration::stk_io
{
auto CommandGenerator::toString() const -> std::string
{
    std::stringstream tStringStream;
    tStringStream << "generated:";
    tStringStream << std::setprecision(mPrecision);
    tStringStream << mElements.mX << "x" << mElements.mY << "x" << mElements.mZ;
    tStringStream << "|bbox:" << mLowerBounds.x << "," << mLowerBounds.y << "," << mLowerBounds.z << ",";
    tStringStream << mUpperBounds.x << "," << mUpperBounds.y << "," << mUpperBounds.z;
    if (mType == CommandElementType::kTet)
    {
        tStringStream << "|tets";
    }
    tStringStream << nodesetString();
    tStringStream << sidesetString();
    return tStringStream.str();
}

auto CommandGenerator::volume() const -> double
{
    return (mUpperBounds.x - mLowerBounds.x) * (mUpperBounds.y - mLowerBounds.y) * (mUpperBounds.z - mLowerBounds.z);
}

auto CommandGenerator::numberOfElements() const -> unsigned int
{
    const unsigned int tHexTotalElements = mElements.mX * mElements.mY * mElements.mZ;
    constexpr unsigned int tTetsPerHex = 6;

    if (mType == CommandElementType::kHex)
    {
        return tHexTotalElements;
    }

    return tHexTotalElements * tTetsPerHex;
}

auto CommandGenerator::numberOfNodes() const -> unsigned int
{
    return (mElements.mX + 1) * (mElements.mY + 1) * (mElements.mZ + 1);
}

auto CommandGenerator::nodesetString() const -> std::string
{
    const std::string tString = detail::xyz_boundary_string(mNodeset);
    if (!tString.empty())
    {
        return "|nodeset:" + tString;
    }
    return {};
}

auto CommandGenerator::sidesetString() const -> std::string
{
    const std::string tString = detail::xyz_boundary_string(mSideset);
    if (!tString.empty())
    {
        return "|sideset:" + tString;
    }
    return {};
}

namespace detail
{
auto xyz_boundary_string(const NodeSetSideSetIdentifiers& aNodeSetSideSetIdentifiers) -> std::string
{
    return std::string(aNodeSetSideSetIdentifiers.mLowerX.mValue ? "x" : "") +
           std::string(aNodeSetSideSetIdentifiers.mUpperX.mValue ? "X" : "") +
           std::string(aNodeSetSideSetIdentifiers.mLowerY.mValue ? "y" : "") +
           std::string(aNodeSetSideSetIdentifiers.mUpperY.mValue ? "Y" : "") +
           std::string(aNodeSetSideSetIdentifiers.mLowerZ.mValue ? "z" : "") +
           std::string(aNodeSetSideSetIdentifiers.mUpperZ.mValue ? "Z" : "");
}

}  // namespace detail

}  // namespace plato::third_party_integration::stk_io
