#ifndef PLATO_DOCUMENTATION_DATATYPE
#define PLATO_DOCUMENTATION_DATATYPE

namespace plato::documentation
{
/// @brief Base template that is specialized for providing documentation strings for data types, such as FileName.
template <typename T>
struct DataTypeDocumentation
{
};

template <>
struct DataTypeDocumentation<double>
{
    static constexpr auto mDocumentation = std::string_view{"Floating point"};
};

template <>
struct DataTypeDocumentation<int>
{
    static constexpr auto mDocumentation = std::string_view{"Signed integer"};
};

template <>
struct DataTypeDocumentation<unsigned int>
{
    static constexpr auto mDocumentation = std::string_view{"Unsigned integer"};
};

template <>
struct DataTypeDocumentation<bool>
{
    static constexpr auto mDocumentation = std::string_view{"`true`|`false`"};
};

}  // namespace plato::documentation

#endif
