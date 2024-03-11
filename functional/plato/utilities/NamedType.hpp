#ifndef PLATO_UTILITIES_NAMEDTYPE
#define PLATO_UTILITIES_NAMEDTYPE

#include <utility>

namespace plato::utilities
{
/// @brief A strongly-typed wrapper for clarifying interfaces.
///
/// The purpose of this class is to facilitate using strong types for making
/// function signatures clear. This can help catch errors at compile time that would
/// normally be caught at run time. For example, if we have a function for loading an
/// xml file into an object, we might need the file name and a node name as string arguments:
/// @code{.cpp}
/// void loadXML(const std::string& aFileName, const std::string& aNodeName);
/// @endcode
/// However, this interface is error-prone since it is easy to swap the arguments.
/// Instead, this wrapper may be used, for example:
/// @code{.cpp}
/// using XMLFileName = NamedType<std::string, struct XMLFileNameTag>;
/// using XMLNodeName = NamedType<std::string, struct XMLNodeNameTag>;
/// void loadXML(const XMLFileName& aFileName, const XMLNodeName& aNodeName);
/// @endcode
/// This involves more code, but prevents errors because now `loadXML` can only be called
/// with the strong types, such as:
/// @code{.cpp}
/// loadXML(XMLFileName{"myXmlFile.xml"}, XMLNodeName{"TopNode"});
/// @endcode
///
/// @tparam T The wrapped type.
/// @tparam NamedTag A tag to associate with the type, typically an empty struct with a meaningful
///  name shown in the example. This prevents copying two NamedType with the same wrapped type T.
template <typename T, typename NameTag>
struct NamedType
{
    constexpr explicit NamedType(const T& value) : mValue(value) {}
    constexpr explicit NamedType(T&& value) : mValue(std::move(value)) {}

    T mValue;
};

}  // namespace plato::utilities
#endif