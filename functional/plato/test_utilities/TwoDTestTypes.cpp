#include "plato/test_utilities/TwoDTestTypes.hpp"

namespace plato::test_utilities
{
double TwoDVector::operator()(const unsigned int index) const { return mData[index]; }

double TwoDMatrix::operator()(const unsigned int i, const unsigned int j) const { return mData[i][j]; }

TwoDVector makeTwoDVector(const double x0, const double x1) { return TwoDVector{{x0, x1}}; }

TwoDMatrix makeTwoDMatrix(const double x00, const double x01, const double x10, const double x11)
{
    return TwoDMatrix{std::array<double, 2>{x00, x01}, std::array<double, 2>{x10, x11}};
}

TwoDVector operator*(const TwoDVector& x, const TwoDMatrix& A)
{
    return makeTwoDVector(x(0) * A(0, 0) + x(1) * A(1, 0), x(0) * A(0, 1) + x(1) * A(1, 1));
}

TwoDVector operator*(double a, const TwoDVector& x) { return makeTwoDVector(a * x(0), a * x(1)); }

TwoDVector operator+(const TwoDVector& x, const TwoDVector& y) { return makeTwoDVector(x(0) + y(0), x(1) + y(1)); }

bool operator==(const TwoDVector& x, const TwoDVector& y) { return x.mData == y.mData; }

bool operator==(const TwoDMatrix& x, const TwoDMatrix& y) { return x.mData == y.mData; }

std::ostream& operator<<(std::ostream& stream, const TwoDVector& x)
{
    stream << "[" << x(0) << ", " << x(1) << "]";
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const TwoDMatrix& x)
{
    stream << "[" << x(0, 0) << ", " << x(0, 1) << "]\n";
    stream << "[" << x(1, 0) << ", " << x(1, 1) << "]";
    return stream;
}

}  // namespace plato::test_utilities
