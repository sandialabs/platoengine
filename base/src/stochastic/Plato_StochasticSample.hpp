#pragma once

#include "Plato_NamedType.hpp"

#include <vector>

namespace Plato
{
/// @brief Strong type for a Sample, which is a vector of doubles.
/// The purpose of using a strong type is so we can specialize `std::hash`
/// without specializing for all `std::vector<double>`.
using Sample = Plato::Utils::NamedType<std::vector<double>, struct SampleTag>;

[[nodiscard]]
bool operator==(const Sample& aLHS, const Sample& aRHS);

[[nodiscard]]
std::size_t vector_hash(const std::vector<double>& aVector);
}

template<>
struct std::hash<Plato::Sample>
{
    std::size_t operator()(const Plato::Sample& aSample) const
    {
        return Plato::vector_hash(aSample.mValue);
    }
};
