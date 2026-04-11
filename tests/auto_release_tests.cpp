#include <gtest/gtest.h>

#include "utils/auto_release.h"

namespace arm
{

TEST(AutoRelease, DefaultConstructedIsInvalid)
{
    const auto ar = AutoRelease<int>{};
    EXPECT_FALSE(static_cast<bool>(ar));
    EXPECT_EQ(ar.get(), 0);
}

TEST(AutoRelease, ConstructedWithValueIsValid)
{
    auto deleted = false;
    {
        auto ar = AutoRelease<int>{42, [&deleted](int) { deleted = true; }};
        EXPECT_TRUE(static_cast<bool>(ar));
        EXPECT_EQ(ar.get(), 42);
        EXPECT_FALSE(deleted);
    }
    EXPECT_TRUE(deleted);
}

TEST(AutoRelease, DeleterReceivesWrappedValue)
{
    auto received = int{};
    {
        auto ar = AutoRelease<int>{7, [&received](int v) { received = v; }};
    }
    EXPECT_EQ(received, 7);
}

TEST(AutoRelease, DefaultConstructedDeleterNotCalled)
{
    // Destroying a default-constructed AutoRelease must not invoke any deleter.
    auto called = false;
    {
        auto ar = AutoRelease<int>{};
        // Assign a deleter after default construction via move-assignment
        // to confirm the invalid guard still suppresses the call.
        ar = AutoRelease<int>{0, [&called](int) { called = true; }};
        // ar holds value 0, which is the Invalid sentinel — deleter must not fire.
    }
    EXPECT_FALSE(called);
}

TEST(AutoRelease, MoveConstructorTransfersOwnership)
{
    auto delete_count = int{0};
    auto deleter = [&delete_count](int) { ++delete_count; };

    auto a = AutoRelease<int>{1, deleter};
    {
        auto b = AutoRelease<int>{std::move(a)};
        EXPECT_EQ(delete_count, 0);
        EXPECT_FALSE(static_cast<bool>(a)); // a should be emptied
        EXPECT_TRUE(static_cast<bool>(b));
    }
    // Only b's destructor should fire the deleter.
    EXPECT_EQ(delete_count, 1);
}

TEST(AutoRelease, MoveAssignmentTransfersOwnership)
{
    auto delete_count = int{0};
    auto deleter = [&delete_count](int) { ++delete_count; };

    auto a = AutoRelease<int>{2, deleter};
    {
        auto b = AutoRelease<int>{};
        b = std::move(a);
        EXPECT_EQ(delete_count, 0);
        EXPECT_FALSE(static_cast<bool>(a));
        EXPECT_TRUE(static_cast<bool>(b));
    }
    EXPECT_EQ(delete_count, 1);
}

TEST(AutoRelease, ResetCallsOldDeleterAndSetsNewValue)
{
    auto delete_count = int{0};
    auto ar = AutoRelease<int>{10, [&delete_count](int) { ++delete_count; }};

    ar.reset(20);

    EXPECT_EQ(delete_count, 1);
    EXPECT_EQ(ar.get(), 20);
}

TEST(AutoRelease, ImplicitConversionToUnderlyingType)
{
    auto ar = AutoRelease<int>{99, [](int) {}};
    const int val = ar;
    EXPECT_EQ(val, 99);
}

} // namespace arm
