#ifndef PLATO_INPUT_PARSER_CROSSREFERENCE
#define PLATO_INPUT_PARSER_CROSSREFERENCE

#include <any>
#include <boost/spirit/include/qi.hpp>
#include <string>

#include "plato/input_parser/ComponentType.hpp"

namespace plato::input_parser
{

/// @brief A type-erased wrapper for input structs, which may be of any type.
class CrossReferencedInput
{
   public:
    CrossReferencedInput() = default;

    template <typename T, typename = std::enable_if<!std::is_convertible_v<T, CrossReferencedInput>>>
    explicit CrossReferencedInput(T&& aInitialValue);

    /// @brief Returns the held object
    /// @pre holdsExpectedType must return `true` for type @a T.
    template <typename T>
    [[nodiscard]] auto get() const -> const T&;

    template <typename T>
    [[nodiscard]] auto get() -> T&;

    /// @brief Assigns a new value with @a aValue.
    template <typename T>
    void set(T&& aValue);

    /// @brief Checks that the held object has type @a T.
    template <typename T>
    [[nodiscard]] auto holdsExpectedType() const -> bool;

    /// @brief Checks that the held object has a value.
    [[nodiscard]] auto hasValue() const -> bool;

   private:
    std::any mInput;
};

/// @brief Helper for parsing a cross-referenced input block
/// Use this type in the input structs for a cross referenced block
/// @todo Fix name
template <ComponentType kComponentType>
struct NewCrossReference
{
    using value_type = char;
    [[nodiscard]] std::string::const_iterator begin() const { return mName.begin(); }
    [[nodiscard]] std::string::const_iterator end() const { return mName.end(); }
    [[nodiscard]] std::string::iterator begin() { return mName.begin(); }
    [[nodiscard]] std::string::iterator end() { return mName.end(); }
    void insert(std::string::iterator aIter, char aVal) { mName.insert(aIter, aVal); }

    std::string mName;
    CrossReferencedInput mInputBlock;
    constexpr static inline ComponentType mComponentType = kComponentType;
};

template <typename T, typename>
CrossReferencedInput::CrossReferencedInput(T&& aInitialValue) : mInput{std::forward<T>(aInitialValue)}
{
}

template <typename T>
auto CrossReferencedInput::get() const -> const T&
{
    assert(holdsExpectedType<T>());
    return std::any_cast<const T&>(mInput);
}

template <typename T>
auto CrossReferencedInput::get() -> T&
{
    assert(holdsExpectedType<T>());
    return std::any_cast<T&>(mInput);
}

template <typename T>
void CrossReferencedInput::set(T&& aValue)
{
    mInput = std::forward<T>(aValue);
}

template <typename T>
auto CrossReferencedInput::holdsExpectedType() const -> bool
{
    return mInput.type() == typeid(T);
}
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <plato::input_parser::ComponentType kComponentType>
struct create_parser<plato::input_parser::NewCrossReference<kComponentType>>
{
    typedef proto::result_of::deep_copy<BOOST_TYPEOF((qi::lexeme[+qi::graph]))>::type type;

    static type call() { return proto::deep_copy((qi::lexeme[+qi::graph])); }
};
}  // namespace boost::spirit::traits
#endif
