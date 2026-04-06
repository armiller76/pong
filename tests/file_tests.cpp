#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "engine/file.h"
#include "utils/exception.h"

namespace pong
{

namespace
{

class TempFile
{
  public:
    explicit TempFile(std::string_view name)
        : path_(std::filesystem::temp_directory_path() / std::filesystem::path{name})
    {
    }

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    auto path() const -> const std::filesystem::path &
    {
        return path_;
    }

  private:
    std::filesystem::path path_;
};

auto write_bytes(const std::filesystem::path &path, std::span<const std::byte> bytes) -> void
{
    auto out = std::ofstream(path, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(out.is_open());
    out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    ASSERT_TRUE(static_cast<bool>(out));
}

} // namespace

TEST(File, MissingFileThrows)
{
    const auto missing = std::filesystem::temp_directory_path() / "pong_missing_file_test.bin";
    std::filesystem::remove(missing);

    EXPECT_THROW((File{missing}), arm::Exception);
}

TEST(File, EmptyFileLoadsAndReportsEmpty)
{
    auto tmp = TempFile{"pong_empty_file_test.bin"};
    write_bytes(tmp.path(), {});

    auto file = File::read_binary(tmp.path());
    EXPECT_TRUE(file.empty());
    EXPECT_EQ(file.size(), 0u);
}

TEST(File, ReadBinaryRoundTripsBytes)
{
    auto tmp = TempFile{"pong_read_binary_test.bin"};
    const auto bytes = std::array{
        std::byte{0x10},
        std::byte{0x20},
        std::byte{0x30},
        std::byte{0x40},
    };
    write_bytes(tmp.path(), bytes);

    auto file = File::read_binary(tmp.path());
    EXPECT_EQ(file.size(), bytes.size());
    EXPECT_EQ(file.data().size(), bytes.size());
    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        EXPECT_EQ(file.data()[i], bytes[i]);
    }
}

TEST(File, ValidateSpirvRejectsInvalidMagic)
{
    auto tmp = TempFile{"pong_invalid_spirv_magic.spv"};
    const auto words = std::array<std::uint32_t, 2>{0xDEADBEEFu, 1u};
    write_bytes(tmp.path(), std::as_bytes(std::span<const std::uint32_t>{words}));

    auto file = File::read_binary(tmp.path());
    EXPECT_FALSE(file.validate_spirv());
    EXPECT_THROW((File::read_shader(tmp.path())), arm::Exception);
}

TEST(File, ValidateSpirvRejectsMisalignedData)
{
    auto tmp = TempFile{"pong_invalid_spirv_alignment.spv"};
    const auto bytes = std::array{
        std::byte{0x03},
        std::byte{0x02},
        std::byte{0x23},
        std::byte{0x07},
        std::byte{0x01},
    };
    write_bytes(tmp.path(), bytes);

    auto file = File::read_binary(tmp.path());
    EXPECT_FALSE(file.validate_spirv());
    EXPECT_THROW((file.as_spirv()), arm::Exception);
}

TEST(File, AsSpirvReturnsWordViewForValidShaderBlob)
{
    auto tmp = TempFile{"pong_valid_spirv_blob.spv"};
    const auto words = std::array<std::uint32_t, 4>{
        0x07230203u,
        0x00010000u,
        0u,
        0u,
    };
    write_bytes(tmp.path(), std::as_bytes(std::span<const std::uint32_t>{words}));

    auto file = File::read_shader(tmp.path());
    ASSERT_TRUE(file.validate_spirv());

    auto view = file.as_spirv();
    ASSERT_EQ(view.size(), words.size());
    for (std::size_t i = 0; i < words.size(); ++i)
    {
        EXPECT_EQ(view[i], words[i]);
    }
}

} // namespace pong
