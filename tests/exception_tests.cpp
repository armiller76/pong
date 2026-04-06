#include <string>

#include <gtest/gtest.h>

#include "utils/exception.h"

namespace pong
{

TEST(Exception, WhatReturnsFormattedMessage)
{
    const auto e = arm::Exception{"hello {} {}", "pong", 42};
    EXPECT_EQ(e.what(), std::string{"hello pong 42"});
}

TEST(Exception, ToStringContainsMessage)
{
    const auto e = arm::Exception{"bad {}", "news"};
    const auto text = e.to_string();

    EXPECT_NE(text.find("bad news"), std::string::npos);
}

} // namespace pong
