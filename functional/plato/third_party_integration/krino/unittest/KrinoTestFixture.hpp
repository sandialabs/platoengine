#include <gtest/gtest.h>
#include <mpi.h>

#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino::unittest
{

class KrinoTestFixture : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            initialize_environment_for_krino(MPI_COMM_WORLD);
            tFirstTime = false;
        }
    }
};
}  // namespace plato::third_party_integration::krino::unittest
