#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "engine/resource_manager.h"
#include "engine/vulkan/vulkan_gpu_buffer.h"
#include "engine/vulkan/vulkan_gpu_image.h"
#include "engine/vulkan/vulkan_utils.h"
#include "graphics/image.h"
#include "graphics/texture2d.h"
#include "graphics/texture2d_impl_vulkan.h"
#include "utils/exception.h"
#include "utils/hash.h"

namespace pong
{

namespace
{

using UploadGpuImageSignature = void (VulkanGpuImage::*)(VulkanImmediateCommandContext &, const Image &);
using UploadTextureImplSignature = void (Texture2DImpl_Vulkan::*)(VulkanImmediateCommandContext &, const Image &);
using UploadTextureSignature = void (Texture2D::*)(VulkanImmediateCommandContext &, const Image &);

static_assert(std::is_same_v<decltype(&VulkanGpuImage::upload), UploadGpuImageSignature>);
static_assert(std::is_same_v<decltype(&Texture2DImpl_Vulkan::upload), UploadTextureImplSignature>);
static_assert(std::is_same_v<decltype(&Texture2D::upload_pixels), UploadTextureSignature>);

} // namespace

TEST(Image, StoresWhiteRgba8Pixel)
{
    constexpr auto white = std::array<std::uint8_t, 4>{255u, 255u, 255u, 255u};
    const auto image = Image{"white_1x1", Extent2D{1u, 1u}, ImageFormat::RGBA8, white};

    EXPECT_EQ(image.name(), "white_1x1");
    EXPECT_EQ(image.extent().width, 1u);
    EXPECT_EQ(image.extent().height, 1u);
    EXPECT_EQ(image.format(), ImageFormat::RGBA8);
    ASSERT_EQ(image.pixels().size(), 4u);
    EXPECT_EQ(image.pixels()[0], 255u);
    EXPECT_EQ(image.pixels()[1], 255u);
    EXPECT_EQ(image.pixels()[2], 255u);
    EXPECT_EQ(image.pixels()[3], 255u);
}

TEST(Image, RejectsZeroExtent)
{
    constexpr auto white = std::array<std::uint8_t, 4>{255u, 255u, 255u, 255u};
    EXPECT_THROW((Image{"bad_extent", Extent2D{0u, 1u}, ImageFormat::RGBA8, white}), arm::Exception);
}

TEST(Image, RejectsUndefinedFormat)
{
    constexpr auto white = std::array<std::uint8_t, 4>{255u, 255u, 255u, 255u};
    EXPECT_THROW((Image{"bad_format", Extent2D{1u, 1u}, ImageFormat::UNDEFINED, white}), arm::Exception);
}

TEST(Image, RejectsEmptyPixels)
{
    constexpr auto none = std::array<std::uint8_t, 0>{};
    EXPECT_THROW((Image{"empty_pixels", Extent2D{1u, 1u}, ImageFormat::RGBA8, none}), arm::Exception);
}

TEST(Image, BytesPerPixelForCommonFormats)
{
    EXPECT_EQ(bytes_per_pixel(ImageFormat::R8), 1u);
    EXPECT_EQ(bytes_per_pixel(ImageFormat::RG8), 2u);
    EXPECT_EQ(bytes_per_pixel(ImageFormat::RGB8), 3u);
    EXPECT_EQ(bytes_per_pixel(ImageFormat::RGBA8), 4u);
    EXPECT_EQ(bytes_per_pixel(ImageFormat::RGBA16F), 8u);
}

TEST(Image, BytesPerPixelUndefinedThrows)
{
    EXPECT_THROW((void)bytes_per_pixel(ImageFormat::UNDEFINED), arm::Exception);
}

TEST(Image, ToStringRepresentativeFormats)
{
    EXPECT_EQ(to_string(ImageFormat::R8), "R8");
    EXPECT_EQ(to_string(ImageFormat::RGB8), "RGB8");
    EXPECT_EQ(to_string(ImageFormat::RGBA8), "RGBA8");
    EXPECT_EQ(to_string(ImageFormat::BC7), "BC7");
}

TEST(Image, FloatingFormatsMapToExpectedVulkanFormats)
{
    EXPECT_EQ(to_vk(ImageFormat::RGBA16F), ::vk::Format::eR16G16B16A16Sfloat);
    EXPECT_EQ(to_vk(ImageFormat::RGBA32F), ::vk::Format::eR32G32B32A32Sfloat);

    EXPECT_EQ(to_pong(::vk::Format::eR16G16B16A16Sfloat), ImageFormat::RGBA16F);
    EXPECT_EQ(to_pong(::vk::Format::eR32G32B32A32Sfloat), ImageFormat::RGBA32F);
}

TEST(Image, FloatingFormatsHaveExpectedBytesPerPixel)
{
    EXPECT_EQ(bytes_per_pixel(ImageFormat::RGBA16F), 8u);
    EXPECT_EQ(bytes_per_pixel(ImageFormat::RGBA32F), 16u);
}

TEST(Image, FloatingFormatPayloadSizeMatchesDimensions)
{
    const auto extent = Extent2D{17u, 31u};

    const auto size_16f =
        static_cast<std::size_t>(extent.width) * extent.height * bytes_per_pixel(ImageFormat::RGBA16F);
    const auto size_32f =
        static_cast<std::size_t>(extent.width) * extent.height * bytes_per_pixel(ImageFormat::RGBA32F);

    auto pixels_16f = std::vector<std::uint8_t>(size_16f, 0x3Cu);
    auto pixels_32f = std::vector<std::uint8_t>(size_32f, 0x7Fu);

    const auto image_16f = Image{"float16_tex", extent, ImageFormat::RGBA16F, pixels_16f};
    const auto image_32f = Image{"float32_tex", extent, ImageFormat::RGBA32F, pixels_32f};

    EXPECT_EQ(image_16f.pixels().size(), size_16f);
    EXPECT_EQ(image_32f.pixels().size(), size_32f);
}

TEST(Image, FloatingFormatCopiesPixelPayloadIndependently)
{
    const auto extent = Extent2D{4u, 4u};
    const auto payload_size =
        static_cast<std::size_t>(extent.width) * extent.height * bytes_per_pixel(ImageFormat::RGBA16F);

    auto source = std::vector<std::uint8_t>(payload_size, 0x11u);
    const auto first_before = source.front();

    const auto image = Image{"float_copy_test", extent, ImageFormat::RGBA16F, source};

    source.front() = 0xFFu;
    source.back() = 0xEEu;

    EXPECT_EQ(image.pixels().front(), first_before);
    EXPECT_EQ(image.pixels().size(), payload_size);
}

TEST(Image, FloatingFormatLargeTextureShape)
{
    const auto extent = Extent2D{256u, 256u};
    const auto payload_size =
        static_cast<std::size_t>(extent.width) * extent.height * bytes_per_pixel(ImageFormat::RGBA32F);
    auto payload = std::vector<std::uint8_t>(payload_size, 0x42u);

    const auto image = Image{"float32_large", extent, ImageFormat::RGBA32F, payload};

    EXPECT_EQ(image.extent().width, 256u);
    EXPECT_EQ(image.extent().height, 256u);
    EXPECT_EQ(image.pixels().size(), payload_size);
}

TEST(ApiContracts, TypeOwnershipAndMoveSemantics)
{
    EXPECT_FALSE((std::is_copy_constructible_v<ResourceManager>));
    EXPECT_FALSE((std::is_copy_assignable_v<ResourceManager>));
    EXPECT_FALSE((std::is_move_constructible_v<ResourceManager>));
    EXPECT_FALSE((std::is_move_assignable_v<ResourceManager>));

    EXPECT_FALSE((std::is_copy_constructible_v<Texture2D>));
    EXPECT_FALSE((std::is_copy_assignable_v<Texture2D>));
    EXPECT_TRUE((std::is_move_constructible_v<Texture2D>));
    EXPECT_TRUE((std::is_move_assignable_v<Texture2D>));

    EXPECT_FALSE((std::is_copy_constructible_v<Texture2DImpl_Vulkan>));
    EXPECT_FALSE((std::is_copy_assignable_v<Texture2DImpl_Vulkan>));
    EXPECT_TRUE((std::is_move_constructible_v<Texture2DImpl_Vulkan>));
    EXPECT_TRUE((std::is_move_assignable_v<Texture2DImpl_Vulkan>));

    EXPECT_FALSE((std::is_copy_constructible_v<VulkanGpuImage>));
    EXPECT_FALSE((std::is_copy_assignable_v<VulkanGpuImage>));
    EXPECT_TRUE((std::is_move_constructible_v<VulkanGpuImage>));
    EXPECT_TRUE((std::is_move_assignable_v<VulkanGpuImage>));

    EXPECT_FALSE((std::is_copy_constructible_v<VulkanGpuBuffer>));
    EXPECT_FALSE((std::is_copy_assignable_v<VulkanGpuBuffer>));
    EXPECT_TRUE((std::is_move_constructible_v<VulkanGpuBuffer>));
    EXPECT_TRUE((std::is_move_assignable_v<VulkanGpuBuffer>));
}

} // namespace pong
