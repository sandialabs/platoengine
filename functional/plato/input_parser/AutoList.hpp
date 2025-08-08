#ifndef PLATO_INPUTPARSER_AUTOLIST
#define PLATO_INPUTPARSER_AUTOLIST

#include <boost/spirit/include/qi.hpp>
#include <concepts>
#include <vector>

namespace plato::input_parser
{

template <typename T>
concept ParsableType = requires {
    requires std::is_invocable_v<decltype(boost::spirit::traits::create_parser<T>::call)>;
} || std::is_same_v<T, std::string>;

/// @brief An auto parser for parsing lists of user-defined ParsableTypes.
///
/// @tparam ParsableType A type that provides a specialization to struct create_parser
template <ParsableType ParsableType>
struct AutoList
{
    using value_type = ParsableType;
    using iterator = std::vector<ParsableType>::iterator;
    using const_iterator = std::vector<ParsableType>::const_iterator;

    AutoList() = default;
    AutoList(std::vector<ParsableType> aList);

    [[nodiscard]] iterator begin();
    [[nodiscard]] iterator end();
    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;

    void insert(std::vector<ParsableType>::iterator aIter, ParsableType aVal);

    std::vector<ParsableType> mList;
};

template <ParsableType ParsableType>
AutoList<ParsableType>::AutoList(std::vector<ParsableType> aList) : mList(std::move(aList))
{
}

template <ParsableType ParsableType>
auto AutoList<ParsableType>::begin() -> AutoList<ParsableType>::iterator
{
    return mList.begin();
}

template <ParsableType ParsableType>
auto AutoList<ParsableType>::end() -> AutoList<ParsableType>::iterator
{
    return mList.end();
}

template <ParsableType ParsableType>
auto AutoList<ParsableType>::begin() const -> AutoList<ParsableType>::const_iterator
{
    return mList.cbegin();
}

template <ParsableType ParsableType>
auto AutoList<ParsableType>::end() const -> AutoList<ParsableType>::const_iterator
{
    return mList.cend();
}

template <ParsableType ParsableType>
void AutoList<ParsableType>::insert(std::vector<ParsableType>::iterator aIter, ParsableType aVal)
{
    mList.insert(aIter, std::move(aVal));
}

}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <typename ParsableType>
struct create_parser<plato::input_parser::AutoList<ParsableType>>
{
    static auto call() { return proto::deep_copy((qi::auto_ % ',')); }
    using type = decltype(call());
};

}  // namespace boost::spirit::traits

#endif
