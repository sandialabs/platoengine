#include "Plato_OperationTypes.hpp"
#include "Plato_EnumTable.hpp"
#include "Plato_FreeFunctions.hpp"

#include <cassert>

namespace Plato
{
namespace 
{
const EnumTable<OperationType> kOperationTable({
    {OperationType::kCriterionValue, "CriterionValue"},
    {OperationType::kCriterionGradient, "CriterionGradient"},
    {OperationType::kCriterionHessian, "CriterionHessian"}});
}

std::string operationTypeName(const OperationType aOperation)
{
    const boost::optional<std::string> tName = kOperationTable.toString(aOperation);
    assert(tName);
    return *tName;
}

boost::optional<OperationType> operationTypeIgnoreSpaces(const std::string& aOperation)
{
    return kOperationTable.toEnum(stripSpaces(aOperation));
}

boost::optional<OperationType> operationType(const std::string& aOperation)
{
    return kOperationTable.toEnum(aOperation);
}
}