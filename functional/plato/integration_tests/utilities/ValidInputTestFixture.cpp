#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"

#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
const auto kInput = create_valid_example_input();
}

ValidInputTestFixture::ValidInputTestFixture()
    : FileCreatingTestFixture{kInput.get<input_parser::density_topology>().front().mesh_name.value().mToken},
      mInput{kInput}
{
}

auto ValidInputTestFixture::parsedInput() const -> const input_parser::ParsedInput& { return mInput; }

}  // namespace plato::integration_tests::utilities
