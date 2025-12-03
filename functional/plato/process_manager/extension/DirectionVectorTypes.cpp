#include "plato/process_manager/extension/DirectionVectorTypes.hpp"

#include "plato/third_party_integration/rol/Utilities.hpp"

DEFINE_ENUM_SYMBOL_TABLE(DirectionVectorTypes, plato::input_parser)

namespace plato::process_manager::extension
{
void make_uniform_or_random_perturbation(ROL::Vector<double>& aROLVector,
                                         const input_parser::DirectionVectorTypes aDirectionVectorType)
{
    if (aDirectionVectorType == input_parser::DirectionVectorTypes::kRandom)
    {
        third_party_integration::rol::randomize_and_normalize(aROLVector);
    }
    else if (aDirectionVectorType == input_parser::DirectionVectorTypes::kUniformPositive)
    {
        third_party_integration::rol::make_uniform(aROLVector, 1.0);
    }
    else
    {
        third_party_integration::rol::make_uniform(aROLVector, -1.0);
    }
}
}  // namespace plato::process_manager::extension
