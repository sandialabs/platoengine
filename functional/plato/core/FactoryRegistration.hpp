#ifndef PLATO_CORE_FACTORYREGISTRATION
#define PLATO_CORE_FACTORYREGISTRATION

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace plato::core
{
template <typename Return, typename Input>
using FactoryFunction = std::function<Return(const Input&)>;

/// @brief Object used for static registration of creation functions that construct objects
///  from parsed input data.
///
/// The purpose of this struct is to enable static registration of the functions
/// used in factories to create objects that require parsed user input.
/// The ctor for a FactoryRegistration object is a key-value pair, mapping a string to a function that
/// creates the object the factory is responsible for.
///
/// Client code should provide a type alias specializing this template. For example, registering
/// a new filter is:
/// @code
/// namespace{
/// [[maybe_unused]] static auto kNewFilterRegistration = FilterRegistration{
///   "filter-type", // Must match the name used in the input block
///    [](){ return make_new_filter(); // Must be a function that creates the desired filter
/// };
/// }
/// @endcode
///
/// @tparam FactoryReturn The object type created by the factory.
/// @tparam FactoryInput The type of the input data needed by the factory function as an argument.
template <typename FactoryReturn, typename FactoryInput>
struct FactoryRegistration
{
    FactoryRegistration(std::string aName, FactoryFunction<FactoryReturn, FactoryInput> aFunction);
};

/// @brief Constructs the object registered with @a aFunctionName, using @a aInput.
/// @return If the function is not registered, returns `std::nullopt`.
template <typename FactoryReturn, typename FactoryInput, typename Input>
[[nodiscard]] std::optional<FactoryReturn> create_object_from_factory(const std::string_view aFunctionName,
                                                                      Input&& aInput);

/// @brief Checks if the function labeled with name @a aFunctionName is registered with the
///  factory associated with template types @a FactoryReturn and @a FactoryInput.
template <typename FactoryReturn, typename FactoryInput>
[[nodiscard]] bool is_factory_function_registered(const std::string_view aFunctionName);

namespace detail
{
/// @return Map holding registered functions used to create CriterionFunction objects in the factory.
template <typename FactoryReturn, typename FactoryInput>
[[nodiscard]] auto registered_factory_functions()
    -> std::unordered_map<std::string, FactoryFunction<FactoryReturn, FactoryInput>>&
{
    static auto tFunctions = std::unordered_map<std::string, FactoryFunction<FactoryReturn, FactoryInput>>{};
    return tFunctions;
}

}  // namespace detail

template <typename FactoryReturn, typename FactoryInput>
FactoryRegistration<FactoryReturn, FactoryInput>::FactoryRegistration(
    std::string aName, FactoryFunction<FactoryReturn, FactoryInput> aFunction)
{
    detail::registered_factory_functions<FactoryReturn, FactoryInput>().try_emplace(std::move(aName),
                                                                                    std::move(aFunction));
}

template <typename FactoryReturn, typename FactoryInput, typename Input>
[[nodiscard]] std::optional<FactoryReturn> create_object_from_factory(const std::string_view aFunctionName,
                                                                      Input&& aInput)
{
    if (const auto tIter =
            detail::registered_factory_functions<FactoryReturn, FactoryInput>().find(std::string{aFunctionName});
        tIter != core::detail::registered_factory_functions<FactoryReturn, FactoryInput>().end())
    {
        return tIter->second(std::forward<Input>(aInput));
    }
    else
    {
        return std::nullopt;
    }
}

template <typename FactoryReturn, typename FactoryInput>
bool is_factory_function_registered(const std::string_view aFunctionName)
{
    return detail::registered_factory_functions<FactoryReturn, FactoryInput>().count(std::string{aFunctionName}) == 1;
}

}  // namespace plato::core

#endif
