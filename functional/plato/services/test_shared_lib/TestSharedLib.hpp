#ifndef PLATO_SERVICES_TESTSHAREDLIB_TESTSHAREDLIB
#define PLATO_SERVICES_TESTSHAREDLIB_TESTSHAREDLIB

#include <memory>

namespace plato::services::test_shared_lib
{
/// @brief Rounds @a aValue to the nearest integer.
/// @note This is only for testing shared library loading.
extern "C" auto my_round(double aValue) -> long int;

/// @brief A test interface class for testing SharedLibraryObject.
class TestInterface
{
   public:
    TestInterface() = default;
    virtual ~TestInterface() = default;

    TestInterface(const TestInterface&) = delete;
    TestInterface(TestInterface&&) = delete;
    auto operator=(const TestInterface&) -> TestInterface& = delete;
    auto operator=(TestInterface&&) -> TestInterface& = delete;

    [[nodiscard]] virtual auto testRound() const -> long int = 0;
};

/// @brief Creates an instance of TestRound, a derived class of TestInterface.
extern "C" auto create_test_interface(double aValue) -> std::unique_ptr<TestInterface>;

/// @brief A fake library loading function
extern "C" void load_library();

/// @brief A fake library unloading function
extern "C" void unload_library();

}  // namespace plato::services::test_shared_lib

#endif
