#ifndef PLATO_TEST_UTILITIES_TWODTYPES
#define PLATO_TEST_UTILITIES_TWODTYPES

#include <array>
#include <ostream>

namespace plato::test_utilities
{
struct TwoDVector
{
    [[nodiscard]] double operator()(const unsigned int index) const;
    std::array<double, 2> mData = {0.0, 0.0};
};

struct TwoDMatrix
{
    [[nodiscard]] double operator()(const unsigned int i, const unsigned int j) const;
    std::array<std::array<double, 2>, 2> mData = {std::array<double, 2>{0.0, 0.0}, std::array<double, 2>{0.0, 0.0}};
};

[[nodiscard]] TwoDVector makeTwoDVector(const double x0, const double x1);
[[nodiscard]] TwoDMatrix makeTwoDMatrix(const double x00, const double x01, const double x10, const double x11);

[[nodiscard]] TwoDVector operator*(const TwoDVector& x, const TwoDMatrix& A);
[[nodiscard]] TwoDVector operator*(double a, const TwoDVector& x);
[[nodiscard]] TwoDVector operator+(const TwoDVector& x, const TwoDVector& y);
[[nodiscard]] bool operator==(const TwoDVector& x, const TwoDVector& y);
[[nodiscard]] bool operator==(const TwoDMatrix& x, const TwoDMatrix& y);
std::ostream& operator<<(std::ostream& stream, const TwoDVector& x);
std::ostream& operator<<(std::ostream& stream, const TwoDMatrix& x);

// [y1] = x1 * x2
// [y2] = x1 + x2
struct TwoDVectorFunction
{
    [[nodiscard]] TwoDVector operator()(const TwoDVector& x) const { return makeTwoDVector(x(0) * x(1), x(0) + x(1)); }
};

// [dy1/dx1, dy1/dx2]
// [dy2/dx1, dy2/dx2]
struct TwoDVectorFunctionJacobian
{
    [[nodiscard]] TwoDMatrix operator()(const TwoDVector& x) const { return makeTwoDMatrix(x(1), x(0), 1.0, 1.0); }
};

// y = x1^2 + x2^3
struct TwoDScalarFunction
{
    [[nodiscard]] double operator()(const TwoDVector& x) const { return x(0) * x(0) + x(1) * x(1) * x(1); }
};

// [dy/dx1]
// [dy/dx2]
struct TwoDScalarFunctionGradient
{
    [[nodiscard]] TwoDVector operator()(const TwoDVector& x) const
    {
        return makeTwoDVector(2.0 * x(0), 3 * x(1) * x(1));
    }
};

}  // namespace plato::test_utilities

#endif
