#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_VIEWSIZETYPES
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_VIEWSIZETYPES

#include "plato/utilities/NamedType.hpp"

namespace plato::third_party_integration::snopt
{
using ConstraintSizeType = utilities::NamedType<std::size_t, struct NumberOfConstraintsTag>;
using DesignVariableSizeType = utilities::NamedType<std::size_t, struct NumberOfDesignVariablesTag>;
}  // namespace plato::third_party_integration::snopt

#endif
