#include "plato/test_utilities/TwoDTestTypes.hpp"

#include <array>
#include <ostream>

namespace plato::test_utilities
{
auto TwoDVector::operator()(const unsigned int index) const -> double { return mData.at(index); }

auto TwoDMatrix::operator()(const unsigned int i, const unsigned int j) const -> double { return mData.at(i).at(j); }

auto makeTwoDMatrix(const double x00, const double x01, const double x10, const double x11) -> TwoDMatrix
{
    return TwoDMatrix{std::array<double, 2>{x00, x01}, std::array<double, 2>{x10, x11}};
}

auto transpose(const TwoDMatrix& aTwoDMatrix) -> TwoDMatrix
{
    return makeTwoDMatrix(aTwoDMatrix(0, 0), aTwoDMatrix(1, 0), aTwoDMatrix(0, 1), aTwoDMatrix(1, 1));
}

auto operator*(const TwoDVector& x, const TwoDMatrix& A) -> TwoDVector
{
    return makeTwoDVector(x(0) * A(0, 0) + x(1) * A(1, 0), x(0) * A(0, 1) + x(1) * A(1, 1));
}

auto operator*(double a, const TwoDVector& x) -> TwoDVector { return makeTwoDVector(a * x(0), a * x(1)); }

auto operator*(const TwoDMatrix& A, const TwoDMatrix& B) -> TwoDMatrix
{
    return makeTwoDMatrix(A(0, 0) * B(0, 0) + A(0, 1) * B(1, 0), A(0, 0) * B(0, 1) + A(0, 1) * B(1, 1),
                          A(1, 0) * B(0, 0) + A(1, 1) * B(1, 0), A(1, 0) * B(0, 1) + A(1, 1) * B(1, 1));
}

auto operator+(const TwoDVector& x, const TwoDVector& y) -> TwoDVector
{
    return makeTwoDVector(x(0) + y(0), x(1) + y(1));
}

auto operator==(const TwoDVector& x, const TwoDVector& y) -> bool { return x.mData == y.mData; }

auto operator==(const TwoDMatrix& x, const TwoDMatrix& y) -> bool { return x.mData == y.mData; }

auto operator<<(std::ostream& stream, const TwoDVector& x) -> std::ostream&
{
    stream << "[" << x(0) << ", " << x(1) << "]";
    return stream;
}

auto operator<<(std::ostream& stream, const TwoDMatrix& x) -> std::ostream&
{
    stream << "[" << x(0, 0) << ", " << x(0, 1) << "]\n";
    stream << "[" << x(1, 0) << ", " << x(1, 1) << "]";
    return stream;
}

}  // namespace plato::test_utilities
