#include <gtest/gtest.h>

#include "math/rectangle.h"
#include "platform/win32_window.h"


namespace pong
{

struct TestWindow
{
    Win32Window window{"PongTest", Rectangle{Offset2D{100, 100}, Extent2D{64, 64}}};
};

TEST(window, construct_and_run)
{
    auto test_window = TestWindow();
    EXPECT_FALSE(test_window.window.should_close());
    EXPECT_NE(test_window.window.win32_handles().window, nullptr);
}

TEST(Win32Window, CloseCallbacksFireOnWmClose)
{
    TestWindow test_window;

    int calledA = 0, calledB = 0;
    auto hA = test_window.window.add_close_callback([&] { ++calledA; });
    auto hB = test_window.window.add_close_callback([&] { ++calledB; });
    EXPECT_NE(hA, hB);

    // Simulate WM_CLOSE
    test_window.window.handle_message(nullptr, WM_CLOSE, 0, 0);

    EXPECT_TRUE(test_window.window.should_close());
    EXPECT_EQ(calledA, 1);
    EXPECT_EQ(calledB, 1);

    // Remove one and ensure it no longer fires
    test_window.window.remove_close_callback(hA);
    calledA = 0;
    calledB = 0;
    test_window.window.handle_message(nullptr, WM_CLOSE, 0, 0);
    EXPECT_EQ(calledA, 0);
    EXPECT_EQ(calledB, 1);
}

TEST(Win32Window, ResizeCallbacksFireOnWmSize)
{
    TestWindow test_window;

    int hitCount = 0;
    std::uint32_t lastW = 0, lastH = 0;

    auto h = test_window.window.add_resize_callback(
        [&](std::uint32_t w, std::uint32_t h)
        {
            ++hitCount;
            lastW = w;
            lastH = h;
        });

    // Simulate WM_SIZE (SIZE_RESTORED). lParam packs width/height.
    constexpr std::uint32_t W = 800, H = 600;
    test_window.window.handle_message(nullptr, WM_SIZE, SIZE_RESTORED, MAKELPARAM(W, H));

    EXPECT_EQ(hitCount, 1);
    EXPECT_EQ(lastW, W);
    EXPECT_EQ(lastH, H);

    // SIZE_MINIMIZED should not invoke callbacks
    test_window.window.handle_message(nullptr, WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(1024, 768));
    EXPECT_EQ(hitCount, 1);

    // Removing should stop invocations
    test_window.window.remove_resize_callback(h);
    test_window.window.handle_message(nullptr, WM_SIZE, SIZE_RESTORED, MAKELPARAM(1024, 768));
    EXPECT_EQ(hitCount, 1);
}

TEST(Win32Window, SizePixelsReflectsInternalState)
{
    TestWindow test_window;
    auto s = test_window.window.extent();
    EXPECT_GT(s.width, 0u);
    EXPECT_GT(s.height, 0u);
}

}
