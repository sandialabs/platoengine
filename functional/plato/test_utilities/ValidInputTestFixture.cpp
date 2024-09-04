#include "plato/test_utilities/ValidInputTestFixture.hpp"

#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::test_utilities
{
namespace
{
const auto kInput = create_valid_example_input();
}

ValidInputTestFixture::ValidInputTestFixture()
    : FileCreatingTestFixture{kInput.mDensityTopology->mesh_name.value().mToken}, mInput{kInput}
{
}

const input_parser::ParsedInput& ValidInputTestFixture::parsedInput() const { return mInput; }

}  // namespace plato::test_utilities
