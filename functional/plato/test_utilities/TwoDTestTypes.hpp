#ifndef PLATO_TEST_UTILITIES_TWODTYPES
#define PLATO_TEST_UTILITIES_TWODTYPES

#include <array>
#include <ostream>

namespace plato::test_utilities
{
struct TwoDVector
{
    [[nodiscard]] auto operator()(unsigned int aIndex) const -> double;
    std::array<double, 2> mData = {0.0, 0.0};

    [[nodiscard]] auto operator==(const TwoDVector& aOther) const -> bool = default;
};

struct TwoDMatrix
{
    [[nodiscard]] auto operator()(unsigned int aRowIndex, unsigned int aColumnIndex) const -> double;
    std::array<std::array<double, 2>, 2> mData = {std::array<double, 2>{0.0, 0.0}, std::array<double, 2>{0.0, 0.0}};

    [[nodiscard]] auto operator==(const TwoDMatrix& aOther) const -> bool = default;
};

[[nodiscard]] constexpr auto make_two_d_vector(const double aX0, const double aX1) -> TwoDVector
{
    return TwoDVector{{aX0, aX1}};
}
[[nodiscard]] auto make_two_d_matrix(double aX00, double aX01, double aX10, double aX11) -> TwoDMatrix;
[[nodiscard]] auto transpose(const TwoDMatrix& aTwoDMatrix) -> TwoDMatrix;

[[nodiscard]] auto operator*(const TwoDVector& aVector, const TwoDMatrix& aMatrix) -> TwoDVector;
[[nodiscard]] auto operator*(double aScalar, const TwoDVector& aVector) -> TwoDVector;
[[nodiscard]] auto operator*(const TwoDMatrix& aMatrix0, const TwoDMatrix& aMatrix1) -> TwoDMatrix;
[[nodiscard]] auto operator+(const TwoDVector& aX0, const TwoDVector& aX1) -> TwoDVector;
auto operator<<(std::ostream& aStream, const TwoDVector& aVector) -> std::ostream&;
auto operator<<(std::ostream& aStream, const TwoDMatrix& aMatrix) -> std::ostream&;

// [y1] = x1 * x2
// [y2] = x1 + x2
struct TwoDVectorFunction
{
    [[nodiscard]] auto operator()(const TwoDVector& aVector) const -> TwoDVector
    {
        return make_two_d_vector(aVector(0) * aVector(1), aVector(0) + aVector(1));
    }
};

// [dy1/dx1, dy1/dx2]
// [dy2/dx1, dy2/dx2]
struct TwoDVectorFunctionJacobian
{
    [[nodiscard]] auto operator()(const TwoDVector& aVector) const -> TwoDMatrix
    {
        return make_two_d_matrix(aVector(1), aVector(0), 1.0, 1.0);
    }
};

// [dy1/dx1, dy2/dx1]
// [dy1/dx2, dy2/dx2]
struct TwoDVectorFunctionAdjointJacobian
{
    [[nodiscard]] auto operator()(const TwoDVector& aVector) const -> TwoDMatrix
    {
        return make_two_d_matrix(aVector(1), 1.0, aVector(0), 1.0);
    }
};

// y = x1^2 + x2^3
struct TwoDScalarFunction
{
    [[nodiscard]] auto operator()(const TwoDVector& aVector) const -> double
    {
        return aVector(0) * aVector(0) + aVector(1) * aVector(1) * aVector(1);
    }
};

// [dy/dx1]
// [dy/dx2]
struct TwoDScalarFunctionGradient
{
    [[nodiscard]] auto operator()(const TwoDVector& aVector) const -> TwoDVector
    {
        constexpr auto tSquarePower = 2.0;
        constexpr auto tCubePower = 3.0;
        return make_two_d_vector(tSquarePower * aVector(0), tCubePower * aVector(1) * aVector(1));
    }
};

}  // namespace plato::test_utilities

#endif
