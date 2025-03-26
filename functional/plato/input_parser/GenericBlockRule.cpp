#include "plato/input_parser/GenericBlockRule.hpp"

namespace plato::input_parser
{
[[nodiscard]] auto to_string(const GenericBlockData& aData) -> std::string
{
    return std::accumulate(aData.mInput.begin(), aData.mInput.end(), std::string{},
                           [](std::string aConcatenatedString, const GenericToken& aNewToken)
                           {
                               aConcatenatedString += aNewToken.mToken;
                               aConcatenatedString += " ";
                               return aConcatenatedString;
                           });
}
}  // namespace plato::input_parser
