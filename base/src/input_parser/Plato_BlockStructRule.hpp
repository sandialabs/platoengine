#ifndef PLATO_BLOCKSTRUCTRULE_HPP
#define PLATO_BLOCKSTRUCTRULE_HPP

#include "Plato_EnumParser.hpp"
#include "Plato_InputEnumTypes.hpp"
#include "Plato_InputBlocks.hpp"

#include <boost/spirit/include/qi.hpp>

#include <boost/phoenix.hpp>

#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/at_key.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace Plato
{
namespace bsq = boost::spirit::qi;
namespace bsa = boost::spirit::ascii;

template<typename Iterator>
const bsq::rule<Iterator, std::string(), bsa::space_type> kIdentifierRule = bsq::lexeme[+bsq::graph];
}

namespace Plato{

namespace detail{

template<typename BlockStruct, std::size_t Index>
struct MemberTypeAt
{
    using type = typename boost::fusion::result_of::value_at_c<BlockStruct, Index>::type;
};

template<typename BlockStruct, std::size_t Index>
std::string member_name()
{
    return boost::fusion::extension::struct_member_name<BlockStruct, Index>::call();
}

template<typename Iterator, typename BlockStruct, std::size_t Index>
using RuleAtIndex = boost::spirit::qi::rule<
    Iterator, 
    typename MemberTypeAt<BlockStruct, Index>::type(), 
    boost::spirit::ascii::space_type>;

template<typename Iterator, typename BlockStruct, std::size_t Index>
RuleAtIndex<Iterator, BlockStruct, Index> rule_at_index()
{
    RuleAtIndex<Iterator, BlockStruct, Index> rule = bsq::lit(member_name<BlockStruct, Index>()) > bsq::auto_;
    return rule;
}

template<typename Iterator, typename BlockStruct, std::size_t... Is>
auto rule_tuple_impl(std::integer_sequence<std::size_t, Is...> )
{
    return std::make_tuple(rule_at_index<Iterator, BlockStruct, Is>()...);
}
}

template<typename Iterator, typename BlockStruct>
auto rule_tuple()
{   
    constexpr auto tNumRules = boost::fusion::result_of::size<BlockStruct>::value;
    return detail::rule_tuple_impl<Iterator, BlockStruct>(std::make_index_sequence<tNumRules>{});
}

/// A tuple type containing all key-value-pair rules defined in type BlockStruct
template<typename Iterator, typename BlockStruct>
using BlockRuleTuple = decltype(rule_tuple<Iterator, BlockStruct>());

namespace detail{
template<typename Iterator, typename BlockStruct, typename AllBlockRules, std::size_t... Is>
auto block_or_rule_impl(
    const AllBlockRules& aAllBlockRules,
    std::integer_sequence<std::size_t, Is...> ) -> bsq::rule<Iterator, BlockStruct(), bsa::space_type>
{
    namespace bp = boost::phoenix;
    if constexpr (Plato::Input::kIsNamedBlock<BlockStruct>)
    {
        bsq::rule<Iterator, BlockStruct(), bsa::space_type> tRule =
            kIdentifierRule<Iterator>[bp::at_c<0>(bsq::_val) = bsq::_1] >
        *( (std::get<Is>(aAllBlockRules)[bp::at_c<Is>(bsq::_val) = bsq::_1] | ...) );
        return tRule;
    }
    else
    {
        bsq::rule<Iterator, BlockStruct(), bsa::space_type> tRule =
        *( (std::get<Is>(aAllBlockRules)[bp::at_c<Is>(bsq::_val) = bsq::_1] | ...) );
        return tRule;
    }
}

}

template<typename Iterator, typename BlockStruct, typename AllBlockRules>
auto block_or_rule(const AllBlockRules& aAllBlockRules) -> bsq::rule<Iterator, BlockStruct(), bsa::space_type>
{
    constexpr auto tNumRules = std::tuple_size<AllBlockRules>::value;
    return detail::block_or_rule_impl<Iterator, BlockStruct>(
        aAllBlockRules, 
        std::make_index_sequence<tNumRules>{});
}

/// Generates key-value-pair rules for fields in @a BlockStruct
/// @a BlockStruct must be a struct with public members defined using a boost fusion macro.
template<typename Iterator, typename BlockStruct>
struct BlockStructRule
{
    static constexpr bool kIsNamedBlockStructRule = Plato::Input::kIsNamedBlock<BlockStruct>;

    std::string mBlockType = Input::InputTypeName<BlockStruct>::name;

    bsq::rule<Iterator, void(), bsa::space_type> mPreambleRule = bsq::lit("begin") >> bsq::lit(mBlockType);
    bsq::rule<Iterator, void(), bsa::space_type> mPostambleRule = bsq::lit("end");
    BlockRuleTuple<Iterator, BlockStruct> mAllBlockRules = rule_tuple<Iterator, BlockStruct>();
    bsq::rule<Iterator, BlockStruct(), bsa::space_type> mBlockOrRule = block_or_rule<Iterator, BlockStruct>(mAllBlockRules);

    bsq::rule<Iterator, BlockStruct(), bsa::space_type> mBlockRule = 
          mPreambleRule
        > mBlockOrRule[bsq::_val = bsq::_1]
        > mPostambleRule;
};

}

#endif
