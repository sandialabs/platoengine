#ifndef PLATO_INPUT_PARSER_TEST_HELPERS
#define PLATO_INPUT_PARSER_TEST_HELPERS

#include "Plato_FileList.hpp"

#include <gtest/gtest.h>

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>

#include <string>
#include <vector>

namespace Plato::Test
{
/// @brief Tests that @a aOptionalValue contains a value via assertion, and that it is equal to @a aVal.
template<typename T, typename U>
void test_existence_and_equality(const boost::optional<T>& aOptionalVal, const U& aVal)
{
    ASSERT_TRUE(aOptionalVal);
    EXPECT_EQ(aOptionalVal.value(), aVal);
}

template<>
inline
void test_existence_and_equality(const boost::optional<Plato::FileName>& aOptionalVal, const std::string& aVal)
{
    ASSERT_TRUE(aOptionalVal);
    EXPECT_EQ(aOptionalVal.value().mName, aVal);
}

template<>
inline
void test_existence_and_equality(const boost::optional<Plato::FileList>& aOptionalVal, const std::vector<std::string>& aVal)
{
    ASSERT_TRUE(aOptionalVal);
    EXPECT_EQ(aOptionalVal.value().mList, aVal);
}
}

#endif
