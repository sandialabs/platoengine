#include "plato/services/test_shared_lib/TestSharedLib.hpp"

#include <cmath>
#include <iostream>
#include <memory>

namespace plato::services::test_shared_lib
{
namespace
{
class TestRound : public TestInterface
{
   public:
    TestRound(const double aValue) : mValue{aValue} {}

    [[nodiscard]] auto testRound() const -> long int override { return my_round(mValue); }

   private:
    double mValue;
};
}  // namespace

auto my_round(const double aValue) -> long int { return std::lround(aValue); }

auto create_test_interface(const double aValue) -> std::unique_ptr<TestInterface>
{
    return std::make_unique<TestRound>(aValue);
}

void load_library() { std::cout << "Test library was loaded.\n"; }

void unload_library() { std::cout << "Test library was unloaded.\n"; }

}  // namespace plato::services::test_shared_lib
