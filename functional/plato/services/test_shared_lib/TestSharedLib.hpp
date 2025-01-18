#ifndef PLATO_SERVICES_TESTSHAREDLIB_TESTSHAREDLIB
#define PLATO_SERVICES_TESTSHAREDLIB_TESTSHAREDLIB

#include <memory>

namespace plato::services::test_shared_lib
{
/// @brief Rounds @a aValue to the nearest integer.
/// @note This is only for testing shared library loading.
extern "C" long int my_round(double aValue);

/// @brief A test interface class for testing SharedLibraryObject.
class TestInterface
{
   public:
    virtual ~TestInterface() = default;

    virtual auto testRound() const -> long int = 0;
};

/// @brief Creates an instance of TestRound, a derived class of TestInterface.
extern "C" auto create_test_interface(double aValue) -> std::unique_ptr<TestInterface>;

}  // namespace plato::services::test_shared_lib

#endif
