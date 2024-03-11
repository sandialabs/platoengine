#ifndef PLATO_CORE_VALIDATIONREGISTRATION
#define PLATO_CORE_VALIDATIONREGISTRATION

#include <optional>
#include <string>

#include "plato/input_parser/InputBlocks.hpp"

namespace plato::core
{
template <typename Input>
using ValidationFunction = std::function<std::optional<std::string>(const Input&)>;

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
///   plato::core::ValidationRegistration<input_parser::density_topology>{
///    [](const input_parser::density_topology& aInput){ return validate_foo_parameter(aInput); }
/// };
/// }
/// @endcode
///
/// @tparam ValidationInput The type of the input data needed by the validation function as an argument.
template <typename ValidationInput>
struct ValidationRegistration
{
    ValidationRegistration(ValidationFunction<ValidationInput> aFunction);
    ValidationRegistration(std::initializer_list<ValidationFunction<ValidationInput>> aFunctions);
};

/// @brief Validates @a aInput, appending any error messages to @a aCurrentMessageList and returning
///  the result.
template <typename ValidationInput>
[[nodiscard]] std::vector<std::string> validate(const ValidationInput& aInput,
                                                std::vector<std::string>&& aCurrentMessageList);

namespace detail
{
template <typename ValidationInput>
[[nodiscard]] auto registered_validation_functions() -> std::vector<ValidationFunction<ValidationInput>>&
{
    static auto tFunctions = std::vector<ValidationFunction<ValidationInput>>{};
    return tFunctions;
}

}  // namespace detail

template <typename ValidationInput>
ValidationRegistration<ValidationInput>::ValidationRegistration(ValidationFunction<ValidationInput> aFunction)
{
    detail::registered_validation_functions<ValidationInput>().push_back(std::move(aFunction));
}

template <typename ValidationInput>
ValidationRegistration<ValidationInput>::ValidationRegistration(
    std::initializer_list<ValidationFunction<ValidationInput>> aFunctions)
{
    std::move(aFunctions.begin(), aFunctions.end(),
              std::back_inserter(detail::registered_validation_functions<ValidationInput>()));
}

template <typename ValidationInput>
[[nodiscard]] std::vector<std::string> validate(const ValidationInput& aInput,
                                                std::vector<std::string>&& aCurrentMessageList)
{
    const auto tTests = detail::registered_validation_functions<ValidationInput>();
    for (const auto& iTest : tTests)
    {
        std::optional<std::string> tMessage = iTest(aInput);
        if (tMessage.has_value())
        {
            aCurrentMessageList.emplace_back(std::move(tMessage).value());
        }
    }
    return std::move(aCurrentMessageList);
}

}  // namespace plato::core

#endif
