#include <expected>

#include <gtest/gtest.h>

#include "engine/engine_error.h"

namespace pong
{

TEST(EngineError, CheckVkResultSuccessMapsToOk)
{
    const auto result = check_vk_result(::vk::Result::eSuccess);

    EXPECT_EQ(result.code, ResultCode::Ok);
    EXPECT_TRUE(result.message.empty());
}

TEST(EngineError, CheckVkResultErrorMapsToError)
{
    const auto result = check_vk_result(::vk::Result::eErrorOutOfHostMemory);

    EXPECT_EQ(result.code, ResultCode::Error);
    EXPECT_FALSE(result.message.empty());
}

TEST(EngineError, CheckVkExpectedSuccessForwardsValue)
{
    auto value = std::expected<int, ::vk::Result>{42};
    auto mapped = check_vk_expected(std::move(value));

    ASSERT_TRUE(mapped.has_value());
    EXPECT_EQ(mapped.value(), 42);
}

TEST(EngineError, CheckVkExpectedErrorMapsEngineError)
{
    auto value = std::expected<int, ::vk::Result>{std::unexpected(::vk::Result::eErrorInitializationFailed)};
    auto mapped = check_vk_expected(std::move(value));

    ASSERT_FALSE(mapped.has_value());
    EXPECT_EQ(mapped.error().code, ResultCode::Error);
    EXPECT_FALSE(mapped.error().message.empty());
}

} // namespace pong
