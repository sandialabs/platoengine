#ifndef PLATO_INPUT_PARSER_CROSSREFERENCE
#define PLATO_INPUT_PARSER_CROSSREFERENCE

#include <any>
#include <boost/spirit/include/qi.hpp>
#include <string>

#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/InputBlockData.hpp"

namespace plato::input_parser
{
/// @brief Helper for parsing a cross-referenced input block
/// Use this type in the input structs for a cross referenced block
template <ComponentType kComponentType>
struct CrossReference
{
    using value_type = char;
    [[nodiscard]] auto begin() const -> std::string::const_iterator;
    [[nodiscard]] auto end() const -> std::string::const_iterator;
    [[nodiscard]] auto begin() -> std::string::iterator;
    [[nodiscard]] auto end() -> std::string::iterator;
    void insert(std::string::iterator aIter, char aVal);

    std::string mName;
    InputBlockWrapper mInputBlock;
    constexpr static inline ComponentType mComponentType = kComponentType;
};

template <ComponentType kComponentType>
auto CrossReference<kComponentType>::begin() const -> std::string::const_iterator
{
    return mName.begin();
}

template <ComponentType kComponentType>
auto CrossReference<kComponentType>::end() const -> std::string::const_iterator
{
    return mName.end();
}

template <ComponentType kComponentType>
auto CrossReference<kComponentType>::begin() -> std::string::iterator
{
    return mName.begin();
}

template <ComponentType kComponentType>
auto CrossReference<kComponentType>::end() -> std::string::iterator
{
    return mName.end();
}

template <ComponentType kComponentType>
void CrossReference<kComponentType>::insert(std::string::iterator aIter, char aVal)
{
    mName.insert(aIter, aVal);
}

}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <plato::input_parser::ComponentType kComponentType>
struct create_parser<plato::input_parser::CrossReference<kComponentType>>
{
    typedef proto::result_of::deep_copy<BOOST_TYPEOF((qi::lexeme[+qi::graph]))>::type type;

    static type call() { return proto::deep_copy((qi::lexeme[+qi::graph])); }
};
}  // namespace boost::spirit::traits

#endif
