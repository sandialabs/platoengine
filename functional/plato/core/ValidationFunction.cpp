#include "plato/core/ValidationFunction.hpp"

namespace plato::core
{

auto ValidationFunction::validate(const input_parser::CrossReferencedInput& aInput) const -> std::optional<std::string>
{
    return mValidationFunction(aInput);
}

}  // namespace plato::core
