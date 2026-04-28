#include <gtest/gtest.h>

#include "engine/engine_types.h"
#include "math/rectangle.h"
#include "platform/win32_window.h"
#include "utils/exception.h"

namespace pong
{

struct TestWindow
{
    Win32Window window{RenderContextInfo{
        .project_root = "c:/dev/Pong",
        .app_name = "PongTest",
        .engine_name = "PongTestEngine",
        .frames_in_flight = 2u,
        .clear_color = Color{0.1f, 0.2f, 0.3f, 1.0f},
        .window_rect = Rectangle{Offset2D{100, 100}, Extent2D{64, 64}},
        .version = Version{0u, 0u, 1u}}};
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

    // WM_SIZE only updates internal extent; callbacks are now fired explicitly by caller.
    EXPECT_EQ(hitCount, 0);
    test_window.window.fire_resize_callbacks();
    EXPECT_EQ(hitCount, 1);
    EXPECT_EQ(lastW, W);
    EXPECT_EQ(lastH, H);

    // SIZE_MINIMIZED should not update extent; explicit callback still reports last non-minimized extent.
    test_window.window.handle_message(nullptr, WM_SIZE, SIZE_MINIMIZED, MAKELPARAM(1024, 768));
    test_window.window.fire_resize_callbacks();
    EXPECT_EQ(hitCount, 2);
    EXPECT_EQ(lastW, W);
    EXPECT_EQ(lastH, H);

    // Removing should stop invocations
    test_window.window.remove_resize_callback(h);
    test_window.window.handle_message(nullptr, WM_SIZE, SIZE_RESTORED, MAKELPARAM(1024, 768));
    test_window.window.fire_resize_callbacks();
    EXPECT_EQ(hitCount, 2);
}

TEST(Win32Window, SizePixelsReflectsInternalState)
{
    TestWindow test_window;
    auto s = test_window.window.extent();
    EXPECT_GT(s.width, 0u);
    EXPECT_GT(s.height, 0u);
}

TEST(Win32Window, ResizeDuringSizeMoveIsSuppressedUntilExit)
{
    TestWindow test_window;

    int hit_count = 0;
    auto h = test_window.window.add_resize_callback([&](std::uint32_t, std::uint32_t) { ++hit_count; });

    test_window.window.handle_message(nullptr, WM_ENTERSIZEMOVE, 0, 0);
    test_window.window.handle_message(nullptr, WM_SIZE, SIZE_RESTORED, MAKELPARAM(1200, 700));

    // WM_SIZE updates dimensions and marks resize pending but does not auto-fire callbacks.
    EXPECT_EQ(hit_count, 0);
    EXPECT_TRUE(test_window.window.resize_pending());

    test_window.window.handle_message(nullptr, WM_EXITSIZEMOVE, 0, 0);
    EXPECT_FALSE(test_window.window.resize_pending());

    test_window.window.fire_resize_callbacks();
    EXPECT_EQ(hit_count, 1);

    test_window.window.remove_resize_callback(h);
}

TEST(Win32Window, RemoveCloseCallbackThrowsForUnknownHandle)
{
    TestWindow test_window;
    EXPECT_THROW(test_window.window.remove_close_callback(999999), arm::Exception);
}

TEST(Win32Window, RemoveResizeCallbackThrowsForUnknownHandle)
{
    TestWindow test_window;
    EXPECT_THROW(test_window.window.remove_resize_callback(999999), arm::Exception);
}

}
