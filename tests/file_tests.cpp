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

    auto file = File(tmp.path());
    EXPECT_TRUE(file.empty());
    EXPECT_EQ(file.size(), 0u);
}

TEST(File, LoadsAndRoundTripsBytes)
{
    auto tmp = TempFile{"pong_roundtrip_test.bin"};
    const auto bytes = std::array{
        std::byte{0x10},
        std::byte{0x20},
        std::byte{0x30},
        std::byte{0x40},
    };
    write_bytes(tmp.path(), bytes);

    auto file = File(tmp.path());
    EXPECT_EQ(file.size(), bytes.size());
    EXPECT_EQ(file.data().size(), bytes.size());
    for (std::size_t i = 0; i < bytes.size(); ++i)
    {
        EXPECT_EQ(file.data()[i], bytes[i]);
    }
}

TEST(File, PathAccessor)
{
    auto tmp = TempFile{"pong_path_test.bin"};
    write_bytes(tmp.path(), std::array{std::byte{0xFF}});

    auto file = File(tmp.path());
    EXPECT_EQ(file.path(), tmp.path());
}

TEST(File, DataReturnsConstSpan)
{
    auto tmp = TempFile{"pong_span_test.bin"};
    const auto bytes = std::array{
        std::byte{0xAA},
        std::byte{0xBB},
    };
    write_bytes(tmp.path(), bytes);

    auto file = File(tmp.path());
    auto span = file.data();

    // Verify span properties
    ASSERT_EQ(span.size(), bytes.size());
    EXPECT_EQ(span[0], bytes[0]);
    EXPECT_EQ(span[1], bytes[1]);

    // Span is const, so this shouldn't compile (static assertion in code):
    // span[0] = std::byte{0x00};  // COMPILE ERROR
}

TEST(File, MoveSemantics)
{
    auto tmp = TempFile{"pong_move_test.bin"};
    const auto bytes = std::array{std::byte{0x42}};
    write_bytes(tmp.path(), bytes);

    auto file1 = File(tmp.path());
    EXPECT_EQ(file1.size(), 1u);

    auto file2 = std::move(file1);
    EXPECT_EQ(file2.size(), 1u);
    EXPECT_EQ(file2.data()[0], std::byte{0x42});
}

TEST(File, ExistsStatic)
{
    auto tmp = TempFile{"pong_exists_test.bin"};
    write_bytes(tmp.path(), std::array{std::byte{0x00}});

    EXPECT_TRUE(File::exists(tmp.path()));

    const auto nonexistent = std::filesystem::temp_directory_path() / "pong_does_not_exist_xyz.bin";
    std::filesystem::remove(nonexistent);
    EXPECT_FALSE(File::exists(nonexistent));
}

TEST(File, LargeFile)
{
    auto tmp = TempFile{"pong_large_file_test.bin"};
    const auto size = 1024u * 1024u; // 1 MB
    auto large_data = std::vector<std::byte>(size, std::byte{0x55});
    write_bytes(tmp.path(), large_data);

    auto file = File(tmp.path());
    EXPECT_EQ(file.size(), size);
    EXPECT_EQ(file.data()[0], std::byte{0x55});
    EXPECT_EQ(file.data()[size - 1], std::byte{0x55});
}

} // namespace pong
