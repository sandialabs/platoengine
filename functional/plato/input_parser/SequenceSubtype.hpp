#ifndef PLATO_INPUTPARSER_SEQUENCESUBTYPE
#define PLATO_INPUTPARSER_SEQUENCESUBTYPE

#include <boost/fusion/adapted/struct/detail/extension.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <format>
#include <utility>

#include "plato/documentation/DataTypeDocumentation.hpp"

namespace plato::input_parser
{
template <typename T>
constexpr inline bool kIsInputSequenceSubtype = false;
}

/// @brief A macro to define a specialized input sequence type.
/// This input sequence type can be used to define structs that will always be input by their attribute name followed by
/// a value where all entries are required. An example of this might be a bounded variable where the first attribute
/// could be a name followed by a bounds type.
/// @code{.cpp}
/// PLATO_INPUT_SEQUENCE_SUBTYPE((plato)(input_parser), BoundedVariable, (std::string,
/// name)(plato::input_parser::Bounds, bounds))
/// @endcode
/// This will create a BoundedVariable struct and associated parser that can parse "name TheName bounds [0,1]".
// clang-format off
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define PLATO_INPUT_SEQUENCE_SUBTYPE(NAMESPACE_SEQ, STRUCT_NAME, ATTRIBUTES)                            \
/* NOLINTBEGIN(cppcoreguidelines-special-member-functions,readability-redundant-member-init) */         \
BOOST_FUSION_DEFINE_STRUCT(                                                                             \
    NAMESPACE_SEQ,                                                                                      \
    STRUCT_NAME,                                                                                        \
    ATTRIBUTES                                                                                          \
)                                                                                                       \
/* NOLINTEND(cppcoreguidelines-special-member-functions,readability-redundant-member-init) */           \
namespace plato::input_parser{                                                                          \
template<>                                                                                              \
constexpr inline bool kIsInputSequenceSubtype<STRUCT_NAME> = true;                                      \
                                                                                                        \
[[nodiscard]] inline auto data_type_documentation(const STRUCT_NAME&) -> std::string                    \
{                                                                                                       \
    auto tDocumentation = std::string{"Composite type: "};                                              \
    [&tDocumentation]<std::size_t... kIndices>(const std::index_sequence<kIndices...>)                  \
    {                                                                                                   \
        using plato::documentation::data_type_documentation;                                            \
        (tDocumentation += ... +=                                                                       \
            std::format("`{}` {} ",                                                                     \
                boost::fusion::extension::struct_member_name<STRUCT_NAME, kIndices>::call(),            \
                data_type_documentation(                                                                \
                    typename boost::fusion::result_of::value_at_c<STRUCT_NAME, kIndices>::type{})       \
             ));                                                                                        \
    }(std::make_index_sequence<boost::fusion::result_of::size<STRUCT_NAME>::value>());                  \
    return tDocumentation;                                                                              \
}                                                                                                       \
}                                                                                                       \

// NOLINTEND(cppcoreguidelines-macro-usage)
// clang-format on

namespace boost::spirit::traits
{

///@brief Specialization of the parser for the type PlatoInputSequenceSubtype. Reads in all of the input sequence
/// subtype attributes in order with their struct_member_name followed by the auto for their type.
template <typename PlatoInputSequenceSubtype, typename Iterator>
    requires plato::input_parser::kIsInputSequenceSubtype<PlatoInputSequenceSubtype>
struct create_parser<PlatoInputSequenceSubtype, Iterator>
{
    static auto call()
    {
        return []<std::size_t... kIndices>(const std::index_sequence<kIndices...>)
        {
            return boost::proto::deep_copy(
                ((qi::lit(boost::fusion::extension::struct_member_name<PlatoInputSequenceSubtype, kIndices>::call()) >>
                  qi::auto_) >>
                 ...));
        }(std::make_index_sequence<boost::fusion::result_of::size<PlatoInputSequenceSubtype>::value>());
    }
    using type = decltype(call());
};
}  // namespace boost::spirit::traits

#endif
