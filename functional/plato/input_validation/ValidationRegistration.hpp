#ifndef PLATO_INPUT_VALIDATION_VALIDATIONREGISTRATION
#define PLATO_INPUT_VALIDATION_VALIDATIONREGISTRATION

#include <functional>
#include <optional>
#include <string>

#include "plato/input_validation/ValidationFunction.hpp"

namespace plato::input_parser
{
class NewParsedInput;
class CrossReferencedInput;
}  // namespace plato::input_parser

namespace plato::input_validation
{
/// @brief Object used for static registration of validation functions that validate
///  parsed input data.
///
/// The purpose of this struct is to enable static registration of the functions
/// used in to validate parsed user input of type @a ValidationInput. The ctor can
/// be used with a list of functions, each of which will be added to the full set
/// of validation functions for the templated type.
///
/// To register functions, client code should instantiate a static object in a cpp file.
/// For example, registering a new validation function for density topology is:
/// @code
/// namespace{
/// [[maybe_unused]] static auto kNewValidationRegistration =
///   plato::input_validation::ValidationRegistration<input_parser::density_topology>{
///    [](const input_parser::density_topology& aInput){ return validate_foo_parameter(aInput); }
/// };
/// }
/// @endcode
///
/// @note Registered validation functions should check only one thing at a time to provide the best error messages.
/// @warning Registered validation functions must not assume that any other part of the input is valid, mainly because
/// there is no guarantee of the order of execution of validation functions. If a validation check is dependent on
/// another part of the input, it should check it but exit with no error if it finds an issue with that data, and rely
/// on other checks to provide error messages.
///
/// @tparam ValidationInput The type of the input data needed by the validation function as an argument.
template <typename ValidationInput, typename... AdditionalArgs>
struct ValidationRegistration
{
    ValidationRegistration(ValidationFunction<ValidationInput, AdditionalArgs...> aFunction);
    ValidationRegistration(std::initializer_list<ValidationFunction<ValidationInput, AdditionalArgs...>> aFunctions);
};

/// @brief Convenience alias for registering validation functions for CrossReferencedInput objects.
///
/// This alias can be used for registering validation functions for input blocks of specific types, such as
/// density_topology.
template <typename... AdditionalArgs>
using CrossReferencedInputValidationRegistration =
    ValidationRegistration<input_parser::CrossReferencedInput, AdditionalArgs...>;

/// @brief Convenience alias for registering validation functions for NewParsedInput objects.
///
/// This alias can be used for registering validation functions for the parsed input as a whole, or components as a
/// whole. For example, certain components should only have one definition in an input, such as geometry.
template <typename... AdditionalArgs>
using NewParsedInputValidationRegistration = ValidationRegistration<input_parser::NewParsedInput, AdditionalArgs...>;

/// @brief Validates @a aInput, appending any error messages to @a aCurrentMessageList and returning
///  the result.
template <typename ValidationInput, typename... AdditionalArgs>
[[nodiscard]] auto validate(const ValidationInput& aInput,
                            std::vector<std::string>&& aCurrentMessageList,
                            const AdditionalArgs&...) -> std::vector<std::string>;

namespace detail
{
template <typename ValidationInput, typename... AdditionalArgs>
[[nodiscard]] auto registered_validation_functions()
    -> std::vector<ValidationFunction<ValidationInput, AdditionalArgs...>>&
{
    static auto tFunctions = std::vector<ValidationFunction<ValidationInput, AdditionalArgs...>>{};
    return tFunctions;
}

}  // namespace detail

template <typename ValidationInput, typename... AdditionalArgs>
ValidationRegistration<ValidationInput, AdditionalArgs...>::ValidationRegistration(
    ValidationFunction<ValidationInput, AdditionalArgs...> aFunction)
{
    detail::registered_validation_functions<ValidationInput, AdditionalArgs...>().push_back(std::move(aFunction));
}

template <typename ValidationInput, typename... AdditionalArgs>
ValidationRegistration<ValidationInput, AdditionalArgs...>::ValidationRegistration(
    std::initializer_list<ValidationFunction<ValidationInput, AdditionalArgs...>> aFunctions)
{
    std::move(aFunctions.begin(), aFunctions.end(),
              std::back_inserter(detail::registered_validation_functions<ValidationInput, AdditionalArgs...>()));
}

template <typename ValidationInput, typename... AdditionalArgs>
[[nodiscard]] auto validate(const ValidationInput& aInput,
                            std::vector<std::string>&& aCurrentMessageList,
                            const AdditionalArgs&... aArgs) -> std::vector<std::string>
{
    for (const auto& tTest : detail::registered_validation_functions<ValidationInput, AdditionalArgs...>())
    {
        if (auto tMessage = tTest.validate(aInput, aArgs...))
        {
            aCurrentMessageList.emplace_back(std::move(tMessage).value());
        }
    }
    return std::move(aCurrentMessageList);
}

}  // namespace plato::input_validation

#endif
