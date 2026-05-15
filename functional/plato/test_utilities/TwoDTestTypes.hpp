#ifndef PLATO_TEST_UTILITIES_TWODTYPES
#define PLATO_TEST_UTILITIES_TWODTYPES

#include <array>
#include <ostream>

namespace plato::test_utilities
{
struct TwoDVector
{
    [[nodiscard]] auto operator()(const unsigned int index) const -> double;
    std::array<double, 2> mData = {0.0, 0.0};
};

struct TwoDMatrix
{
    [[nodiscard]] auto operator()(const unsigned int i, const unsigned int j) const -> double;
    std::array<std::array<double, 2>, 2> mData = {std::array<double, 2>{0.0, 0.0}, std::array<double, 2>{0.0, 0.0}};
};

[[nodiscard]] constexpr auto makeTwoDVector(const double x0, const double x1) -> TwoDVector
{
    return TwoDVector{{x0, x1}};
}
[[nodiscard]] auto makeTwoDMatrix(const double x00, const double x01, const double x10, const double x11) -> TwoDMatrix;
[[nodiscard]] auto transpose(const TwoDMatrix& aTwoDMatrix) -> TwoDMatrix;

[[nodiscard]] auto operator*(const TwoDVector& x, const TwoDMatrix& A) -> TwoDVector;
[[nodiscard]] auto operator*(double a, const TwoDVector& x) -> TwoDVector;
[[nodiscard]] auto operator*(const TwoDMatrix& A, const TwoDMatrix& B) -> TwoDMatrix;
[[nodiscard]] auto operator+(const TwoDVector& x, const TwoDVector& y) -> TwoDVector;
[[nodiscard]] auto operator==(const TwoDVector& x, const TwoDVector& y) -> bool;
[[nodiscard]] auto operator==(const TwoDMatrix& x, const TwoDMatrix& y) -> bool;
auto operator<<(std::ostream& stream, const TwoDVector& x) -> std::ostream&;
auto operator<<(std::ostream& stream, const TwoDMatrix& x) -> std::ostream&;

// [y1] = x1 * x2
// [y2] = x1 + x2
struct TwoDVectorFunction
{
    [[nodiscard]] auto operator()(const TwoDVector& x) const -> TwoDVector
    {
        return makeTwoDVector(x(0) * x(1), x(0) + x(1));
    }
};

// [dy1/dx1, dy1/dx2]
// [dy2/dx1, dy2/dx2]
struct TwoDVectorFunctionJacobian
{
    [[nodiscard]] auto operator()(const TwoDVector& x) const -> TwoDMatrix
    {
        return makeTwoDMatrix(x(1), x(0), 1.0, 1.0);
    }
};

// [dy1/dx1, dy2/dx1]
// [dy1/dx2, dy2/dx2]
struct TwoDVectorFunctionAdjointJacobian
{
    [[nodiscard]] auto operator()(const TwoDVector& x) const -> TwoDMatrix
    {
        return makeTwoDMatrix(x(1), 1.0, x(0), 1.0);
    }
};

// y = x1^2 + x2^3
struct TwoDScalarFunction
{
    [[nodiscard]] auto operator()(const TwoDVector& x) const -> double { return x(0) * x(0) + x(1) * x(1) * x(1); }
};

// [dy/dx1]
// [dy/dx2]
struct TwoDScalarFunctionGradient
{
    [[nodiscard]] auto operator()(const TwoDVector& x) const -> TwoDVector
    {
        constexpr auto tSquarePower = 2.0;
        constexpr auto tCubePower = 3.0;
        return makeTwoDVector(tSquarePower * x(0), tCubePower * x(1) * x(1));
    }
};

}  // namespace plato::test_utilities

#endif
