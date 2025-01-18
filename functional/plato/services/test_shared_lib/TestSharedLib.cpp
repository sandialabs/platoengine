#include "plato/services/test_shared_lib/TestSharedLib.hpp"

#include <cmath>

namespace plato::services::test_shared_lib
{
namespace
{
class TestRound : public TestInterface
{
   public:
    TestRound(const double aValue) : mValue{aValue} {}

    auto testRound() const -> long int override { return my_round(mValue); }

   private:
    double mValue;
};
}  // namespace

long int my_round(const double aValue) { return std::lround(aValue); }

auto create_test_interface(const double aValue) -> std::unique_ptr<TestInterface>
{
    return std::make_unique<TestRound>(aValue);
}

}  // namespace plato::services::test_shared_lib
