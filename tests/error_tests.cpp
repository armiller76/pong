#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "utils/error.h"
#include "utils/exception.h"

namespace pong
{

TEST(Ensure, TruePredicateDoesNotThrow)
{
    EXPECT_NO_THROW(arm::ensure(true, "should not fail"));
}

TEST(Ensure, FalsePredicateThrowsException)
{
    EXPECT_THROW((arm::ensure(false, "failure {}", 7)), arm::Exception);

    try
    {
        arm::ensure(false, "failure {}", 7);
        FAIL() << "Expected arm::Exception";
    }
    catch (const arm::Exception &e)
    {
        EXPECT_EQ(e.what(), std::string{"failure 7"});
    }
}

TEST(Ensure, UniquePtrOverloadChecksNullability)
{
    auto ptr = std::unique_ptr<int>{};
    EXPECT_THROW((arm::ensure(ptr, "pointer missing")), arm::Exception);

    ptr = std::make_unique<int>(42);
    EXPECT_NO_THROW(arm::ensure(ptr, "pointer missing"));
}

} // namespace pong
