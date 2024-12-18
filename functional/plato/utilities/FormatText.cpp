#include "plato/utilities/FormatText.hpp"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/tokenized.hpp>
#include <boost/regex.hpp>
#include <cassert>
#include <iostream>
#include <string>

namespace plato::utilities
{

void word_block_justify(const std::string& aString,
                        const TextWidth aWidth,
                        const TextIndent aIndent,
                        std::ostream& aOutputStream)
{
    assert(aString.length() > 0);
    assert(aWidth.mValue > 0);
    assert(aIndent.mValue > 0);
    std::string tIndent(aIndent.mValue, ' ');
    boost::regex tPattern(".{1," + std::to_string(aWidth.mValue) + "}(?=[\\s,;:.!?\\t\\n]|$|[-–—]{1,})");
    auto tTokens = aString | boost::adaptors::tokenized(tPattern);
    std::for_each(tTokens.begin(), tTokens.end(),
                  [&tIndent, &aOutputStream](const auto& aToken) { aOutputStream << tIndent << aToken << "\n"; });
}
}  // namespace plato::utilities
