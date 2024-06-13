#ifndef PLATO_TEST_UTLITIES_TESTCONTEXT
#define PLATO_TEST_UTLITIES_TESTCONTEXT

#include <stack>
#include <string>

/// Convenience macro for instantiating a TestContext object. The file and line are added automatically.
#define TEST_CONTEXT(MESSAGE)                   \
    plato::test_utilities::extend_test_context( \
        plato::test_utilities::TestContext::SingleTestContext{__FILE__, __LINE__, (MESSAGE)})

/// Convenience macro for extending a TestContext object by adding a new context. The file and line are added
/// automatically.
#define EXTEND_CONTEXT(MESSAGE, EXISTING_CONTEXT) \
    plato::test_utilities::extend_test_context(   \
        plato::test_utilities::TestContext::SingleTestContext{__FILE__, __LINE__, (MESSAGE)}, (EXISTING_CONTEXT))

namespace plato::test_utilities
{
/// @brief A helper for adding additional information to test failures.
///
/// This is meant to be used in common test functions that include a gtest test assertion macro.
/// One issue with functions that use test macros is that when a failure occurs, the failing line is
/// within the function. This can obscure the source of the failure, especially when one test
/// calls multiple functions that contain test assertions.
///
/// This is meant to be streamed into the test assertion as:
/// @code{.cpp}
/// void test_things(const TestContext& aTestContext) {
///   // Do stuff
///   EXPECT_EQ(thing1, thing2) << aTestContext;
/// }
/// @endcode
/// This should then be used with the `TEST_CONTEXT` macro as:
/// @code{.cpp}
/// test_things(TEST_CONTEXT("Descriptive error message"));
/// @endcode
/// or directly with the file and line numbers (`__FILE__` and `__LINE__`) as:
/// @code{.cpp}
/// test_things(TestContext{__FILE__, __LINE__, "Descriptive error message"});
/// @endcode
///
/// To add multiple test contexts for multiple function calls, use EXTEND_CONTEXT as
/// @code{.cpp}
/// test_more_things(EXTEND_CONTEXT("Another descriptive error message", aExistingContext));
/// @endcode
struct TestContext
{
    struct SingleTestContext
    {
        std::string mFileName;
        unsigned int mLineNumber;
        std::string mMessage;
    };

    std::stack<SingleTestContext> mTestContexts;
};

[[nodiscard]] inline TestContext extend_test_context(const TestContext::SingleTestContext& aNewContext,
                                                     TestContext aExistingContext = TestContext{})
{
    aExistingContext.mTestContexts.push(aNewContext);
    return aExistingContext;
}

template <typename OutStream>
OutStream& operator<<(OutStream& aOutStream, const TestContext::SingleTestContext& aSingleTestContext)
{
    aOutStream << "Source file: " << aSingleTestContext.mFileName << ":" << aSingleTestContext.mLineNumber << ": "
               << aSingleTestContext.mMessage;
    return aOutStream;
}

template <typename OutStream>
OutStream& operator<<(OutStream& aOutStream, TestContext aTestContext)
{
    while (!aTestContext.mTestContexts.empty())
    {
        aOutStream << aTestContext.mTestContexts.top() << "\n";
        aTestContext.mTestContexts.pop();
    }
    return aOutStream;
}

}  // namespace plato::test_utilities

#endif
