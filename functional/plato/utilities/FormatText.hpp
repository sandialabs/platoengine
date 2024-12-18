#ifndef PLATO_UTILITIES_FORMATTEXT
#define PLATO_UTILITIES_FORMATTEXT

#include <iostream>
#include <string>

#include "plato/utilities/NamedType.hpp"

namespace plato::utilities
{

using TextWidth = NamedType<unsigned long int, struct TextWidthTag>;
using TextIndent = NamedType<unsigned long int, struct TextIndentTag>;
/// @brief Take a string @a aString and split it up by white space so that it fits within a @a aWidth starting at some
/// @a aIndent and puts it in the output stream @a aOutputStream
/// @note This can be replaced with std::fmt when we go to c++20
void word_block_justify(const std::string& aString,
                        const TextWidth aWidth,
                        const TextIndent aIndent,
                        std::ostream& aOutputStream);
}  // namespace plato::utilities

#endif
