#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_UNITTEST_KRINOTESTFIXTURE
#define PLATO_THIRDPARTYINTEGRATION_KRINO_UNITTEST_KRINOTESTFIXTURE

#include <gtest/gtest.h>
#include <mpi.h>

#include <filesystem>

#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{
const std::string_view kLogFile = "Krino_Test.txt";
}

class KrinoTestFixture : virtual public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            initialize_environment_for_krino(kLogFile, MPI_COMM_WORLD);
            tFirstTime = false;
        }
    }
    void TearDown() override { std::filesystem::remove(std::filesystem::path{kLogFile}); }
};
}  // namespace plato::third_party_integration::krino::unittest

#endif
