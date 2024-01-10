#ifndef PLATO_OPERATIONTYPES_HPP_
#define PLATO_OPERATIONTYPES_HPP_

#include <string>

#include <boost/optional.hpp>

namespace Plato
{
enum class OperationType{
  kCriterionValue,
  kCriterionGradient,
  kCriterionHessian
};

std::string operationTypeName(OperationType aOperation);
/// Finds the matching enum, ignoring any spaces in @a aOperation. This is to be used when an
/// operation is two or more words and spaces don't matter. Unfortunately, it's not smart enough
/// to check for spaces only at the beginning, end, or between words, so `Crit er ionV alue` matches
/// `kCriterionValue`.
boost::optional<OperationType> operationTypeIgnoreSpaces(const std::string& aOperation);
boost::optional<OperationType> operationType(const std::string& aOperation);
}

#endif