#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

#include <iomanip>
#include <sstream>
#include <string>

namespace plato::third_party_integration::stk_io
{
std::string CommandGenerator::toString() const
{
    std::stringstream tStringStream;
    tStringStream << "generated:";
    tStringStream << std::setprecision(mPrecision);
    tStringStream << mElements.mX << "x" << mElements.mY << "x" << mElements.mZ;
    tStringStream << "|bbox:" << mLowerBounds.x << "," << mLowerBounds.y << "," << mLowerBounds.z << ",";
    tStringStream << mUpperBounds.x << "," << mUpperBounds.y << "," << mUpperBounds.z;
    if (mType == CommandElementType::Tet)
    {
        tStringStream << "|tets";
    }
    tStringStream << nodesetString();
    tStringStream << sidesetString();
    return tStringStream.str();
}

double CommandGenerator::volume() const
{
    return (mUpperBounds.x - mLowerBounds.x) * (mUpperBounds.y - mLowerBounds.y) * (mUpperBounds.z - mLowerBounds.z);
}

unsigned int CommandGenerator::numberOfElements() const
{
    const unsigned int tHexTotalElements = mElements.mX * mElements.mY * mElements.mZ;
    constexpr unsigned int tTetsPerHex = 6;

    if (mType == CommandElementType::Hex)
    {
        return tHexTotalElements;
    }
    else
    {
        return tHexTotalElements * tTetsPerHex;
    }
}

unsigned int CommandGenerator::numberOfNodes() const
{
    return (mElements.mX + 1) * (mElements.mY + 1) * (mElements.mZ + 1);
}

std::string CommandGenerator::nodesetString() const
{
    const std::string tString = detail::xyz_boundary_string(mNodeset);
    if (!tString.empty())
    {
        return "|nodeset:" + tString;
    }
    return {};
}

std::string CommandGenerator::sidesetString() const
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
std::string xyz_boundary_string(const NodeSetSideSetIdentifiers& aNodeSetSideSetIdentifiers)
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
