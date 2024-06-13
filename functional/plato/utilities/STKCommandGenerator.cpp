#include "STKCommandGenerator.hpp"

#include <iomanip>
#include <sstream>
#include <string>

namespace plato::utilities
{
std::string STKCommandGenerator::toString(const int aPrecision) const
{
    std::stringstream tStringStream;
    tStringStream << "generated:";
    tStringStream << std::setprecision(aPrecision);
    tStringStream << mElements.mX << "x" << mElements.mY << "x" << mElements.mZ;
    tStringStream << "|bbox:" << mLowerBounds.mX << "," << mLowerBounds.mY << "," << mLowerBounds.mZ << ",";
    tStringStream << mUpperBounds.mX << "," << mUpperBounds.mY << "," << mUpperBounds.mZ;
    if (mType == STKCommandElementType::Tet)
    {
        tStringStream << "|tets";
    }
    return tStringStream.str();
}

double STKCommandGenerator::volume() const
{
    return (mUpperBounds.mX - mLowerBounds.mX) * (mUpperBounds.mY - mLowerBounds.mY) *
           (mUpperBounds.mZ - mLowerBounds.mZ);
}

unsigned int STKCommandGenerator::numberOfElements() const
{
    const unsigned int tHexTotalElements = mElements.mX * mElements.mY * mElements.mZ;
    constexpr unsigned int tTetsPerHex = 6;

    if (mType == STKCommandElementType::Hex)
    {
        return tHexTotalElements;
    }
    else
    {
        return tHexTotalElements * tTetsPerHex;
    }
}

unsigned int STKCommandGenerator::numberOfNodes() const
{
    return (mElements.mX + 1) * (mElements.mY + 1) * (mElements.mZ + 1);
}

}  // namespace plato::utilities
