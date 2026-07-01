#ifndef PLATO_DOCUMENTATION_DATATYPE
#define PLATO_DOCUMENTATION_DATATYPE

namespace plato::documentation
{
/// @brief Data type documentation for `double`
[[nodiscard]] auto data_type_documentation(double /*unused*/) -> std::string;

/// @brief Data type documentation for `int`
[[nodiscard]] auto data_type_documentation(int /*unused*/) -> std::string;

/// @brief Data type documentation for `unsigned int`
[[nodiscard]] auto data_type_documentation(unsigned int /*unused*/) -> std::string;

/// @brief Data type documentation for `bool`
[[nodiscard]] auto data_type_documentation(bool /*unused*/) -> std::string;

}  // namespace plato::documentation

#endif
