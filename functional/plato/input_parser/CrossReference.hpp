#ifndef PLATO_INPUT_PARSER_CROSSREFERENCE
#define PLATO_INPUT_PARSER_CROSSREFERENCE

#include <any>
#include <boost/spirit/include/qi.hpp>
#include <string>

namespace plato::input_parser
{

/// @brief Interface for accessing std::any that holds input block in CrossReference
class CrossReferencedInput
{
   public:
    /// @brief Returns the held object
    /// @pre holds_expected_type must return `true` for type @a T.
    template <typename T>
    [[nodiscard]] T get() const
    {
        return std::any_cast<T>(mInput);
    }

    /// @brief Assigns a new value with @a aValue.
    template <typename T>
    void set(T&& aValue)
    {
        mInput = std::forward<T>(aValue);
    }

    /// @brief Checks that the held object has type @a T.
    template <typename T>
    [[nodiscard]] bool holds_expected_type() const
    {
        return mInput.type() == typeid(T);
    }

    /// @brief Checks that the held object has a value.
    [[nodiscard]] bool has_value() const { return mInput.has_value(); }

   private:
    std::any mInput;
};

/// @brief Helper for parsing a cross-referenced input block
/// Use this type in the input structs for a cross referenced block
template <template <typename> typename IsVariantMember>
struct CrossReference
{
    template <typename T>
    using IsVariantType = IsVariantMember<T>;

    using value_type = char;
    [[nodiscard]] std::string::const_iterator begin() const { return mName.begin(); }
    [[nodiscard]] std::string::const_iterator end() const { return mName.end(); }
    [[nodiscard]] std::string::iterator begin() { return mName.begin(); }
    [[nodiscard]] std::string::iterator end() { return mName.end(); }
    void insert(std::string::iterator aIter, char aVal) { mName.insert(aIter, aVal); }

    std::string mName;
    CrossReferencedInput mInputBlock;
};
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <template <typename> typename IsVariantMember>
struct create_parser<plato::input_parser::CrossReference<IsVariantMember>>
{
    typedef proto::result_of::deep_copy<BOOST_TYPEOF((qi::lexeme[+qi::graph]))>::type type;

    static type call() { return proto::deep_copy((qi::lexeme[+qi::graph])); }
};

}  // namespace boost::spirit::traits
#endif