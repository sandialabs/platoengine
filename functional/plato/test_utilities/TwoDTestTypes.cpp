#include "plato/test_utilities/TwoDTestTypes.hpp"

#include <array>
#include <ostream>

namespace plato::test_utilities
{
auto TwoDVector::operator()(const unsigned int aIndex) const -> double { return mData.at(aIndex); }

auto TwoDMatrix::operator()(const unsigned int aRowIndex, const unsigned int aColumnIndex) const -> double
{
    return mData.at(aRowIndex).at(aColumnIndex);
}

auto make_two_d_matrix(const double aX00, const double aX01, const double aX10, const double aX11) -> TwoDMatrix
{
    return TwoDMatrix{std::array<double, 2>{aX00, aX01}, std::array<double, 2>{aX10, aX11}};
}

auto transpose(const TwoDMatrix& aTwoDMatrix) -> TwoDMatrix
{
    return make_two_d_matrix(aTwoDMatrix(0, 0), aTwoDMatrix(1, 0), aTwoDMatrix(0, 1), aTwoDMatrix(1, 1));
}

auto operator*(const TwoDVector& aVector, const TwoDMatrix& aMatrix) -> TwoDVector
{
    return make_two_d_vector(aVector(0) * aMatrix(0, 0) + aVector(1) * aMatrix(1, 0),
                             aVector(0) * aMatrix(0, 1) + aVector(1) * aMatrix(1, 1));
}

auto operator*(const double aScalar, const TwoDVector& aVector) -> TwoDVector
{
    return make_two_d_vector(aScalar * aVector(0), aScalar * aVector(1));
}

auto operator*(const TwoDMatrix& aMatrix0, const TwoDMatrix& aMatrix1) -> TwoDMatrix
{
    return make_two_d_matrix(aMatrix0(0, 0) * aMatrix1(0, 0) + aMatrix0(0, 1) * aMatrix1(1, 0),
                             aMatrix0(0, 0) * aMatrix1(0, 1) + aMatrix0(0, 1) * aMatrix1(1, 1),
                             aMatrix0(1, 0) * aMatrix1(0, 0) + aMatrix0(1, 1) * aMatrix1(1, 0),
                             aMatrix0(1, 0) * aMatrix1(0, 1) + aMatrix0(1, 1) * aMatrix1(1, 1));
}

auto operator+(const TwoDVector& aX0, const TwoDVector& aX1) -> TwoDVector
{
    return make_two_d_vector(aX0(0) + aX1(0), aX0(1) + aX1(1));
}

auto operator<<(std::ostream& aStream, const TwoDVector& aVector) -> std::ostream&
{
    aStream << "[" << aVector(0) << ", " << aVector(1) << "]";
    return aStream;
}

auto operator<<(std::ostream& aStream, const TwoDMatrix& aMatrix) -> std::ostream&
{
    aStream << "[" << aMatrix(0, 0) << ", " << aMatrix(0, 1) << "]\n";
    aStream << "[" << aMatrix(1, 0) << ", " << aMatrix(1, 1) << "]";
    return aStream;
}

}  // namespace plato::test_utilities
