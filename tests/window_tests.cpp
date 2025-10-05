#include "platform/win32_window.h"
#include <gtest/gtest.h>

TEST(window, default_construct)
{
    pong::Win32WindowCreateInfo info;
    EXPECT_EQ(info.x, 0);
    EXPECT_EQ(info.y, 0);
    EXPECT_EQ(info.width, 800);
    EXPECT_EQ(info.height, 600);
}

TEST(window, construct_and_run)
{
    pong::Win32WindowCreateInfo info(100, 100, 640, 480);
    pong::Win32Window window("TestApp", info);
    EXPECT_TRUE(window.running());
    EXPECT_NE(window.instance(), nullptr);
    EXPECT_NE(window.handles().window, nullptr);
}
